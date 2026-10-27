#pragma once

#include "ScanTypes.h"
#include <QObject>
#include <QVector>
#include <atomic>

class CleanupManager : public QObject {
    Q_OBJECT
public:
    explicit CleanupManager(QObject* parent = nullptr);

    bool isBusy() const { return m_busy.load(); }

public slots:
    void run(const QVector<CleanupItem>& items, bool revalidate);
    void cancel();

signals:
    void itemResult(const CleanupResult& result);
    void finishedCleanup(int successCount, qint64 bytesFreed, int failCount);

private:
    CleanupResult processOne(const CleanupItem& item, bool revalidate) const;

    std::atomic_bool m_cancel{false};
    std::atomic_bool m_busy{false};
};
