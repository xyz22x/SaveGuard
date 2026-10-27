#pragma once

#include "ScanTypes.h"
#include <QWidget>

class QTableWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QProgressBar;
class QLabel;
class QPushButton;

class BigFilesPage : public QWidget {
    Q_OBJECT
public:
    explicit BigFilesPage(QWidget* parent = nullptr);

    void applyRecords(const QVector<FileRecord>& records, const ScanStats& stats);
    void setScanBusy(bool busy);
    void setPaused(bool paused);
    void retranslate();
    ScanConfig buildConfig() const;

signals:
    void scanRequested(const ScanConfig& config);
    void cleanupRequested(const QVector<CleanupItem>& items);

private slots:
    void startScan();
    void reviewSelection();
    void applyFilters();

private:
    void buildUi();
    void retranslateTable();
    void updateSelectedLabel();
    void showContextMenu(const QPoint& pos);

    QComboBox* m_threshold = nullptr;
    QSpinBox* m_customSize = nullptr;
    QLineEdit* m_search = nullptr;
    QComboBox* m_extFilter = nullptr;
    QTableWidget* m_table = nullptr;
    QProgressBar* m_progress = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_selected = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QPushButton* m_reviewBtn = nullptr;
    QVector<FileRecord> m_records;
    qint64 m_totalBytes = 0;
    bool m_busy = false;
};
