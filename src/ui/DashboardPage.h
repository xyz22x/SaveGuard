#pragma once

#include "ScanTypes.h"
#include <QWidget>

class QTableWidget;
class QLabel;
class QPushButton;
class QListWidget;

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);

    void refreshDrives();
    void applyScanState(const ScanStats& stats, const QDateTime& finishedAt);
    void setRecentHistory(const QVector<HistoryEntry>& entries);
    void retranslate();

signals:
    void navigateTo(int page);
    void rescanRequested();

private:
    void buildUi();
    void retranslateStatic();

    QTableWidget* m_drives = nullptr;
    QLabel* m_statFiles = nullptr;
    QLabel* m_statGroups = nullptr;
    QLabel* m_statRecoverable = nullptr;
    QLabel* m_lastScan = nullptr;
    QPushButton* m_rescan = nullptr;
    QListWidget* m_recent = nullptr;
};
