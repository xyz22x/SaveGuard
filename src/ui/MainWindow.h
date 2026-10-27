#pragma once

#include "ScanTypes.h"
#include <QMainWindow>
#include <QThread>
#include <QDateTime>
#include <QSet>

class QStackedWidget;
class QListWidget;
class QLabel;
class QPushButton;
class QProgressBar;
class Scanner;
class CleanupManager;
class DashboardPage;
class DuplicatesPage;
class BigFilesPage;
class HistoryPage;
class SettingsPage;
class HistoryStore;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

    void retranslateAll();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onScanStateChanged(const QString& state);
    void onScanBatch(const QVector<FileRecord>& records);
    void onScanFinished(const ScanStats& stats);
    void onDuplicatesReady(const QVector<DuplicateGroup>& groups);
    void onScanRequested(const ScanConfig& config);
    void onBigScanRequested(const ScanConfig& config);
    void onPauseOrResume();
    void onCancelScan();
    void onCleanupRequested(const QVector<CleanupItem>& items);
    void onCleanupItem(const CleanupResult& result);
    void onCleanupFinished(int success, qint64 bytesFreed, int fail);
    void onPageChanged(int index);
    void switchPage(int index);

private:
    void buildUi();
    void connectPages();
    void startScan(const ScanConfig& config);
    void setScanBusy(bool busy);
    void refreshHistory();
    void addHistoryEntry(const HistoryEntry& entry);
    void updateTopState(const QString& stateKey);

    QThread* m_scanThread = nullptr;
    QThread* m_cleanupThread = nullptr;
    Scanner* m_scanner = nullptr;
    CleanupManager* m_cleaner = nullptr;
    HistoryStore* m_history = nullptr;

    QListWidget* m_nav = nullptr;
    QStackedWidget* m_pages = nullptr;
    QLabel* m_topTitle = nullptr;
    QLabel* m_topStatus = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    DashboardPage* m_dashboard = nullptr;
    DuplicatesPage* m_duplicates = nullptr;
    BigFilesPage* m_bigFiles = nullptr;
    HistoryPage* m_historyPage = nullptr;
    SettingsPage* m_settingsPage = nullptr;

    QVector<FileRecord> m_lastRecords;
    ScanStats m_lastStats;
    QDateTime m_lastScanFinishedAt;
    QVector<CleanupItem> m_pendingCleanup;
    QStringList m_cleanupFailures;
    QString m_activeRoots;
    ScanMode m_activeMode = ScanMode::Duplicates;
    bool m_scanBusy = false;
    bool m_cleanupBusy = false;
};
