#include "CleanupManager.h"

#include "Hasher.h"
#include "ProtectedPaths.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>

#include <filesystem>

namespace fs = std::filesystem;

CleanupManager::CleanupManager(QObject* parent) : QObject(parent) {}

void CleanupManager::cancel() { m_cancel.store(true); }

void CleanupManager::run(const QVector<CleanupItem>& items, bool revalidate) {
    if (m_busy.exchange(true)) {
        emit finishedCleanup(0, 0, 0);
        return;
    }
    m_cancel.store(false);

    int success = 0;
    int fail = 0;
    qint64 freed = 0;

    for (const CleanupItem& item : items) {
        if (m_cancel.load()) break;
        const CleanupResult result = processOne(item, revalidate);
        if (result.success) {
            success++;
            freed += item.size;
        } else {
            fail++;
        }
        emit itemResult(result);
    }

    m_busy.store(false);
    emit finishedCleanup(success, freed, fail);
}

CleanupResult CleanupManager::processOne(const CleanupItem& item, bool revalidate) const {
    CleanupResult result;
    result.path = item.path;

    const QString native = QDir::toNativeSeparators(item.path);
    QFileInfo info(native);
    if (!info.exists() || info.isDir()) {
        result.error = QStringLiteral("cleanup_err_missing");
        return result;
    }

    std::error_code ec;
    const fs::path fp = toFsPath(native);
    const fs::file_status lst = fs::symlink_status(fp, ec);
    if (ec || lst.type() == fs::file_type::symlink || lst.type() == fs::file_type::junction) {
        result.error = QStringLiteral("cleanup_err_symlink");
        return result;
    }

    if (ProtectedPaths::isProtected(item.path)) {
        result.error = QStringLiteral("cleanup_err_protected");
        return result;
    }

    if (revalidate) {
        const qint64 currentSize = info.size();
        const qint64 currentMtime = info.lastModified().toSecsSinceEpoch();
        if (currentSize != item.size) {
            result.error = QStringLiteral("cleanup_err_changed_size");
            return result;
        }
        if (item.mtimeSec > 0 && currentMtime != item.mtimeSec) {
            result.error = QStringLiteral("cleanup_err_changed_mtime");
            return result;
        }
        if (!item.miniHash.isEmpty()) {
            QByteArray current;
            const Hasher::Status st = Hasher::mini(native, &current);
            if (st != Hasher::Status::Ok) {
                result.error = QStringLiteral("cleanup_err_read");
                return result;
            }
            if (current != item.miniHash) {
                result.error = QStringLiteral("cleanup_err_changed_content");
                return result;
            }
        }
    }

    if (!QFile::moveToTrash(native)) {
        result.error = QStringLiteral("cleanup_err_trash");
        return result;
    }

    if (QFileInfo::exists(native)) {
        result.error = QStringLiteral("cleanup_err_trash");
        return result;
    }

    result.success = true;
    return result;
}
