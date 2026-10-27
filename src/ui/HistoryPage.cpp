#include "HistoryPage.h"

#include "I18n.h"
#include "Theme.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

HistoryPage::HistoryPage(QWidget* parent) : QWidget(parent) {
    buildUi();
    retranslate();
}

void HistoryPage::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    auto* title = new QLabel(this);
    Theme::setClass(title, "h1");
    title->setObjectName("histTitle");

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() { showDetails(); });

    m_details = new QPlainTextEdit(this);
    m_details->setReadOnly(true);
    m_details->setMaximumHeight(120);
    Theme::setClass(m_details, "details");

    auto* bottom = new QHBoxLayout;
    bottom->addStretch(1);
    m_clearBtn = new QPushButton(this);
    Theme::setClass(m_clearBtn, "danger");
    bottom->addWidget(m_clearBtn);
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        if (m_entries.isEmpty()) return;
        const auto choice = QMessageBox::question(this, i18n("hist_clear_title"),
                                                  i18n("hist_clear_text"));
        if (choice == QMessageBox::Yes) emit clearRequested();
    });

    layout->addWidget(title);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_details);
    layout->addLayout(bottom);
}

void HistoryPage::retranslate() {
    findChild<QLabel*>("histTitle")->setText(i18n("nav_history"));
    m_clearBtn->setText(i18n("btn_clear"));
    m_table->setHorizontalHeaderLabels({
        i18n("hist_col_date"), i18n("hist_col_dur"), i18n("hist_col_type"), i18n("hist_col_roots"),
        i18n("hist_col_files"), i18n("hist_col_rec"), i18n("hist_col_trashed"), i18n("hist_col_errors")});
    if (m_entries.isEmpty()) showDetails();
}

void HistoryPage::setHistory(const QVector<HistoryEntry>& entries) {
    m_entries = entries;
    m_table->setSortingEnabled(false);
    m_table->setRowCount(entries.size());
    for (int i = 0; i < entries.size(); ++i) {
        const HistoryEntry& e = entries[i];
        const QDateTime when = QDateTime::fromMSecsSinceEpoch(e.timestampMs);
        auto* dateItem = new QTableWidgetItem(when.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")));
        dateItem->setData(Qt::UserRole, e.timestampMs);
        m_table->setItem(i, 0, dateItem);
        auto* durItem = new QTableWidgetItem(
            i18n("fmt_duration").arg(int(e.durationMs / 60000)).arg(int((e.durationMs % 60000) / 1000)));
        m_table->setItem(i, 1, durItem);
        const QString type = e.type == QStringLiteral("scan") ? i18n("hist_type_scan")
                             : e.type == QStringLiteral("cleanup") ? i18n("hist_type_cleanup")
                                                                   : i18n("hist_type_scan_big");
        m_table->setItem(i, 2, new QTableWidgetItem(type));
        auto* rootsItem = new QTableWidgetItem(e.roots);
        rootsItem->setToolTip(e.roots);
        m_table->setItem(i, 3, rootsItem);
        auto* filesItem = new QTableWidgetItem(QString::number(e.filesFound));
        filesItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(i, 4, filesItem);
        auto* recItem = new QTableWidgetItem(I18n::formatBytes(e.recoverableBytes));
        recItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(i, 5, recItem);
        auto* trashedItem = new QTableWidgetItem(QString::number(e.trashedCount));
        trashedItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(i, 6, trashedItem);
        auto* errItem = new QTableWidgetItem(QString::number(e.errorCount));
        errItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(i, 7, errItem);
    }
    m_table->setSortingEnabled(true);
    showDetails();
}

void HistoryPage::showDetails() {
    if (m_entries.isEmpty()) {
        m_details->setPlainText(i18n("hist_empty"));
        return;
    }
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_entries.size()) {
        m_details->clear();
        return;
    }
    const HistoryEntry& e = m_entries[row];
    const QDateTime when = QDateTime::fromMSecsSinceEpoch(e.timestampMs);
    QString text = when.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")) + QStringLiteral("\n");
    text += i18n("hist_col_roots") + QStringLiteral(": ") + e.roots + QStringLiteral("\n");
    text += i18n("hist_col_files") + QStringLiteral(": ") + QString::number(e.filesFound)
            + QStringLiteral(" · ") + i18n("dash_recoverable") + QStringLiteral(": ")
            + I18n::formatBytes(e.recoverableBytes) + QStringLiteral("\n");
    text += i18n("hist_col_trashed") + QStringLiteral(": ") + QString::number(e.trashedCount)
            + QStringLiteral(" (") + I18n::formatBytes(e.trashedBytes) + QStringLiteral(") · ")
            + i18n("hist_col_errors") + QStringLiteral(": ") + QString::number(e.errorCount);
    m_details->setPlainText(text);
}
