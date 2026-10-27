#pragma once

#include "ScanTypes.h"
#include <QWidget>

class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QProgressBar;
class QLabel;
class QPushButton;

class DuplicatesPage : public QWidget {
    Q_OBJECT
public:
    explicit DuplicatesPage(QWidget* parent = nullptr);

    void applyGroups(const QVector<DuplicateGroup>& groups, const ScanStats& stats);
    void applyConfig(const ScanConfig& config);
    void setScanBusy(bool busy);
    void setPaused(bool paused);
    void retranslate();
    ScanConfig buildConfig() const;
    QVector<CleanupItem> selectedItems(bool* ok, QString* error) const;
    void removePathsFromResults(const QSet<QString>& paths);
    void clearResults();

signals:
    void scanRequested(const ScanConfig& config);
    void cleanupRequested(const QVector<CleanupItem>& items);

private:
    void buildUi();
    void retranslateTree();
    void updateSummary();
    void showContextMenu(const QPoint& pos);
    void startScan();
    void reviewSelection();

    QListWidget* m_folders = nullptr;
    QTreeWidget* m_tree = nullptr;
    QProgressBar* m_progress = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_summary = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QPushButton* m_reviewBtn = nullptr;
    int m_groupCount = 0;
    bool m_busy = false;
};
