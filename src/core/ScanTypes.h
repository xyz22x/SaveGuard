#pragma once

#include "FileRecord.h"
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMetaType>

enum class ScanMode {
    Duplicates,
    BigFiles
};

struct ScanConfig {
    ScanMode mode = ScanMode::Duplicates;
    QStringList roots;
    QStringList ignoredDirs;
    QStringList ignoredExtensions;
    qint64 minFileSize = 0;
    int maxConcurrent = 4;
};

struct ScanStats {
    qint64 filesSeen = 0;
    qint64 filesIndexed = 0;
    qint64 bytesIndexed = 0;
    qint64 errors = 0;
    qint64 symlinksSkipped = 0;
    qint64 durationMs = 0;
    int duplicateGroups = 0;
    qint64 duplicateSpace = 0;
    QString state;
};

struct DuplicateGroup {
    qint64 size = 0;
    QByteArray hash;
    QVector<FileRecord> files;

    qint64 recoverable() const {
        if (files.size() < 2 || size <= 0) return 0;
        return size * qint64(files.size() - 1);
    }
};

struct CleanupItem {
    QString path;
    qint64 size = 0;
    qint64 mtimeSec = 0;
    QByteArray miniHash;
};

struct CleanupResult {
    QString path;
    bool success = false;
    QString error;
};

struct HistoryEntry {
    qint64 timestampMs = 0;
    qint64 durationMs = 0;
    QString type;
    QString roots;
    qint64 filesFound = 0;
    qint64 recoverableBytes = 0;
    qint64 trashedCount = 0;
    qint64 trashedBytes = 0;
    qint64 errorCount = 0;
};

Q_DECLARE_METATYPE(ScanConfig)
Q_DECLARE_METATYPE(ScanStats)
Q_DECLARE_METATYPE(DuplicateGroup)
Q_DECLARE_METATYPE(CleanupItem)
Q_DECLARE_METATYPE(CleanupResult)
