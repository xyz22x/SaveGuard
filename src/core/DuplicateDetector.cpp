#include "DuplicateDetector.h"
#include "Hasher.h"

#include <QMutex>
#include <QMutexLocker>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace {

void parallelFor(int count, int maxThreads,
                 const std::function<bool()>& shouldContinue,
                 const std::function<void(int)>& body) {
    if (count <= 0) return;
    const int threads = std::max(1, std::min(maxThreads, count));
    std::atomic<int> next{0};
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&]() {
            while (shouldContinue()) {
                const int i = next.fetch_add(1);
                if (i >= count) break;
                body(i);
            }
        });
    }
    for (auto& w : workers) w.join();
}

}

DuplicateDetector::GroupingOutput DuplicateDetector::group(
        std::vector<FileRecord> records, int maxConcurrent,
        const std::function<bool()>& shouldCancel,
        const std::function<bool()>& shouldContinue,
        const std::function<void(int, int)>& hashProgress) {
    GroupingOutput out;

    std::unordered_map<qint64, std::vector<FileRecord>> bySize;
    for (FileRecord& rec : records) bySize[rec.size].push_back(std::move(rec));
    records.clear();

    std::vector<std::pair<qint64, std::vector<FileRecord>*>> candidates;
    for (auto& kv : bySize) {
        if (kv.second.size() >= 2) candidates.push_back({kv.first, &kv.second});
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    qint64 totalToHash = 0;
    for (const auto& c : candidates) totalToHash += qint64(c.second->size());
    std::atomic<qint64> hashErrors{0};
    std::atomic<qint64> hashedDone{0};

    for (const auto& cand : candidates) {
        if (shouldCancel()) {
            out.canceled = true;
            return out;
        }
        std::vector<FileRecord>& files = *cand.second;

        std::mutex miniMutex;
        std::unordered_map<QByteArray, std::vector<FileRecord>> byMini;
        parallelFor(int(files.size()), maxConcurrent, shouldContinue, [&](int i) {
            if (shouldCancel()) return;
            QByteArray mh;
            const Hasher::Status st = Hasher::mini(files[i].path, &mh, shouldCancel);
            QMutexLocker lock(&miniMutex);
            if (st == Hasher::Status::Ok) {
                files[i].miniHash = mh;
                byMini[mh].push_back(std::move(files[i]));
            } else if (st == Hasher::Status::Failed) {
                hashErrors.fetch_add(1, std::memory_order_relaxed);
            }
            const qint64 done = hashedDone.fetch_add(1, std::memory_order_relaxed) + 1;
            if (hashProgress) hashProgress(int(done), int(totalToHash));
        });
        if (shouldCancel()) {
            out.canceled = true;
            return out;
        }

        for (auto& kv : byMini) {
            if (kv.second.size() < 2) continue;
            std::mutex fullMutex;
            std::map<QByteArray, std::vector<FileRecord>> byFull;
            const std::vector<FileRecord> sub = std::move(kv.second);
            parallelFor(int(sub.size()), maxConcurrent, shouldContinue, [&](int i) {
                if (shouldCancel()) return;
                QByteArray fh;
                const Hasher::Status st = Hasher::full(sub[i].path, &fh, shouldCancel);
                QMutexLocker lock(&fullMutex);
                if (st == Hasher::Status::Ok) {
                    byFull[fh].push_back(std::move(sub[i]));
                } else if (st == Hasher::Status::Failed) {
                    hashErrors.fetch_add(1, std::memory_order_relaxed);
                }
                const qint64 done = hashedDone.fetch_add(1, std::memory_order_relaxed) + 1;
                if (hashProgress) hashProgress(int(done), int(totalToHash));
            });
            if (shouldCancel()) {
                out.canceled = true;
                return out;
            }
            for (auto& fk : byFull) {
                if (fk.second.size() < 2) continue;
                DuplicateGroup g;
                g.size = fk.second.front().size;
                g.hash = fk.first;
                g.files = QVector<FileRecord>(fk.second.begin(), fk.second.end());
                std::sort(g.files.begin(), g.files.end(),
                          [](const FileRecord& a, const FileRecord& b) {
                              return a.path.compare(b.path, Qt::CaseInsensitive) < 0;
                          });
                out.groups.append(g);
            }
        }
    }

    std::sort(out.groups.begin(), out.groups.end(),
              [](const DuplicateGroup& a, const DuplicateGroup& b) {
                  if (a.size != b.size) return a.size > b.size;
                  return a.recoverable() > b.recoverable();
              });
    out.hashErrors = hashErrors.load();
    return out;
}
