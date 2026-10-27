#include "Scanner.h"

#include "Hasher.h"
#include "ProtectedPaths.h"

#include <QDir>
#include <QElapsedTimer>

#include <algorithm>
#include <chrono>
#include <deque>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

namespace {

bool isIgnoredExtension(const QString& file, const QStringList& exts) {
    if (exts.isEmpty()) return false;
    const int dot = file.lastIndexOf('.');
    if (dot < 0) return false;
    const QString ext = file.mid(dot).toLower();
    return exts.contains(ext);
}

bool dirIgnored(const QString& name, const QString& absPath, const QStringList& ignored) {
    for (const QString& ig : ignored) {
        const QString n = ig.trimmed();
        if (n.isEmpty()) continue;
        if (name.compare(n, Qt::CaseInsensitive) == 0) return true;
        const QString norm = QDir::fromNativeSeparators(n);
        const QString path = QDir::fromNativeSeparators(absPath);
        if (path.compare(norm, Qt::CaseInsensitive) == 0) return true;
        if (path.startsWith(norm + "/", Qt::CaseInsensitive)) return true;
    }
    return false;
}

qint64 mtimeSeconds(const fs::file_time_type& ftp) {
    const auto sys = std::chrono::clock_cast<std::chrono::system_clock>(ftp);
    return static_cast<qint64>(std::chrono::system_clock::to_time_t(sys));
}

}

Scanner::Scanner(QObject* parent) : QObject(parent) {}

void Scanner::cancel() { m_cancel.store(true); }
void Scanner::pause() { m_pause.store(true); }
void Scanner::resume() { m_pause.store(false); }

bool Scanner::waitIfPaused() {
    while (m_pause.load() && !m_cancel.load()) std::this_thread::sleep_for(std::chrono::milliseconds(120));
    return !m_cancel.load();
}

void Scanner::emitState(const QString& s) {
    m_stateText = s;
    emit stateChanged(s);
}

void Scanner::run(ScanConfig config) {
    if (m_busy.exchange(true)) {
        ScanStats early;
        early.state = QStringLiteral("busy");
        emit finishedStats(early);
        return;
    }
    m_cancel.store(false);
    m_pause.store(false);

    ScanStats stats;
    QElapsedTimer timer;
    timer.start();

    const QStringList exts = config.ignoredExtensions;
    const QStringList dirs = config.ignoredDirs;
    const qint64 minSize = qMax<qint64>(0, config.minFileSize);
    const bool hashes = config.mode == ScanMode::Duplicates;

    emitState(QStringLiteral("scan_state_enumerate"));

    std::vector<FileRecord> all;
    std::vector<FileRecord> pending;
    pending.reserve(256);
    auto flush = [&]() {
        if (!pending.empty()) {
            emit batch(QVector<FileRecord>(pending.begin(), pending.end()));
            all.insert(all.end(), pending.begin(), pending.end());
            pending.clear();
        }
    };

    std::deque<fs::path> stack;
    for (const QString& root : config.roots) {
        const fs::path rp = toFsPath(QDir::fromNativeSeparators(root));
        std::error_code ec;
        if (!fs::exists(rp, ec) || ec) {
            stats.errors++;
            continue;
        }
        stack.push_back(rp);
    }

    while (!stack.empty()) {
        if (!waitIfPaused()) break;
        const fs::path dir = stack.back();
        stack.pop_back();

        const QString dirStr = fromFsPath(dir);
        if (dirIgnored(QDir(dirStr).dirName(), dirStr, dirs)) continue;

        std::error_code ec;
        fs::directory_iterator it(dir, ec);
        if (ec) {
            stats.errors++;
            ec.clear();
            continue;
        }
        for (; it != fs::directory_iterator(); it.increment(ec)) {
            if (ec) {
                stats.errors++;
                ec.clear();
                continue;
            }
            if (m_cancel.load()) break;
            if (!waitIfPaused()) break;

            const fs::path entry = it->path();
            const QString entryStr = fromFsPath(entry);
            const QString name = fromFsPath(entry.filename());

            fs::file_status lst = fs::symlink_status(entry, ec);
            if (ec) {
                stats.errors++;
                ec.clear();
                continue;
            }
            if (lst.type() == fs::file_type::symlink || lst.type() == fs::file_type::junction) {
                stats.symlinksSkipped++;
                continue;
            }
            if (fs::is_directory(lst)) {
                if (ProtectedPaths::isCriticalDir(name, entryStr)) continue;
                stack.push_back(entry);
                continue;
            }
            if (!fs::is_regular_file(lst)) continue;

            stats.filesSeen++;

            std::error_code sec;
            const uintmax_t sz = fs::file_size(entry, sec);
            if (sec) {
                stats.errors++;
                sec.clear();
                continue;
            }
            const fs::file_time_type wt = fs::last_write_time(entry, sec);
            if (sec) {
                stats.errors++;
                sec.clear();
                continue;
            }

            if (static_cast<qint64>(sz) < minSize) continue;
            if (isIgnoredExtension(entryStr, exts)) continue;

            FileRecord rec;
            rec.path = QDir::fromNativeSeparators(entryStr);
            rec.size = static_cast<qint64>(sz);
            rec.mtimeSec = mtimeSeconds(wt);
            stats.filesIndexed++;
            stats.bytesIndexed += rec.size;
            pending.push_back(std::move(rec));
            if (pending.size() >= 256) flush();
        }
        if (m_cancel.load()) break;
    }
    flush();

    if (!m_cancel.load() && hashes) {
        emitState(QStringLiteral("scan_state_hash"));
        auto result = DuplicateDetector::group(
            std::move(all), std::max(1, config.maxConcurrent),
            [this]() { return m_cancel.load(); },
            [this]() { return waitIfPaused(); },
            [this](int done, int total) {
                if (total > 0 && done % 32 == 0) {
                    emitState(QStringLiteral("scan_state_hash:%1:%2").arg(done).arg(total));
                }
            });
        if (result.canceled) {
            stats.state = QStringLiteral("canceled");
        } else {
            emitState(QStringLiteral("scan_state_group"));
            qint64 space = 0;
            for (const DuplicateGroup& g : result.groups) space += g.recoverable();
            stats.duplicateGroups = result.groups.size();
            stats.duplicateSpace = space;
            stats.errors += result.hashErrors;
            emit duplicatesReady(result.groups);
            stats.state = QStringLiteral("done");
        }
    } else if (!m_cancel.load()) {
        stats.state = QStringLiteral("done");
    } else {
        stats.state = QStringLiteral("canceled");
    }

    all.clear();
    stats.durationMs = timer.elapsed();
    m_busy.store(false);
    emit finishedStats(stats);
}
