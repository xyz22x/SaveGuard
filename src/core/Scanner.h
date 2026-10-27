#pragma once

#include "FileRecord.h"
#include "ScanTypes.h"
#include "DuplicateDetector.h"
#include <QObject>
#include <atomic>
#include <vector>

class Scanner : public QObject {
    Q_OBJECT
public:
    explicit Scanner(QObject* parent = nullptr);

    bool isBusy() const { return m_busy.load(); }

public slots:
    void run(ScanConfig config);
    void cancel();
    void pause();
    void resume();

signals:
    void batch(const QVector<FileRecord>& records);
    void stateChanged(const QString& state);
    void duplicatesReady(const QVector<DuplicateGroup>& groups);
    void finishedStats(const ScanStats& stats);

private:
    bool waitIfPaused();
    void emitState(const QString& s);

    std::atomic_bool m_cancel{false};
    std::atomic_bool m_pause{false};
    std::atomic_bool m_busy{false};
    QString m_stateText;
};
