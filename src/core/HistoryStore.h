#pragma once

#include "ScanTypes.h"
#include <QObject>
#include <QString>
#include <QVector>

class HistoryStore {
public:
    explicit HistoryStore(QString filePath);

    QVector<HistoryEntry> entries() const;
    void add(const HistoryEntry& entry);
    void clear();

    static QString defaultFilePath();

private:
    QString m_filePath;
};
