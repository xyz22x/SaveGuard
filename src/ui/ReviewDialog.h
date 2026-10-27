#pragma once

#include "ScanTypes.h"
#include <QDialog>

class QTableWidget;
class QLabel;

class ReviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReviewDialog(const QVector<CleanupItem>& items, QWidget* parent = nullptr);

private:
    QTableWidget* m_table = nullptr;
    QLabel* m_total = nullptr;
};
