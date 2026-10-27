#pragma once

#include "ScanTypes.h"
#include <QWidget>

class QTableWidget;
class QPlainTextEdit;
class QPushButton;

class HistoryPage : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPage(QWidget* parent = nullptr);

    void setHistory(const QVector<HistoryEntry>& entries);
    void retranslate();

signals:
    void clearRequested();

private:
    void buildUi();
    void showDetails();
    QVector<HistoryEntry> m_entries;
    QTableWidget* m_table = nullptr;
    QPlainTextEdit* m_details = nullptr;
    QPushButton* m_clearBtn = nullptr;
};
