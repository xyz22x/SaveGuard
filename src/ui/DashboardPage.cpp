#include "DashboardPage.h"

#include "DrivesInfo.h"
#include "HistoryStore.h"
#include "I18n.h"
#include "Theme.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent) {
    buildUi();
    retranslate();
    refreshDrives();
}

void DashboardPage::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    auto* title = new QLabel(this);
    Theme::setClass(title, "h1");
    title->setObjectName("dashTitle");

    auto* statsRow = new QHBoxLayout;
    statsRow->setSpacing(24);
    auto makeStat = [&](const char* valueObj, const char* labelObj) {
        auto* box = new QVBoxLayout;
        box->setSpacing(2);
        auto* value = new QLabel(this);
        Theme::setClass(value, "statValue");
        value->setObjectName(valueObj);
        auto* label = new QLabel(this);
        Theme::setClass(label, "statLabel");
        label->setObjectName(labelObj);
        box->addWidget(value);
        box->addWidget(label);
        statsRow->addLayout(box);
        return box;
    };
    makeStat("statFiles", "statFilesLabel");
    makeStat("statGroups", "statGroupsLabel");
    makeStat("statRecoverable", "statRecoverableLabel");

    m_lastScan = new QLabel(this);
    Theme::setClass(m_lastScan, "muted");
    m_lastScan->setWordWrap(true);

    m_rescan = new QPushButton(this);
    connect(m_rescan, &QPushButton::clicked, this, &DashboardPage::rescanRequested);

    m_errorsLog = new QPushButton(this);
    m_errorsLog->setObjectName("dashErrorsLog");
    m_errorsLog->setVisible(false);
    connect(m_errorsLog, &QPushButton::clicked, this, &DashboardPage::openErrorsLogRequested);

    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);
    actionRow->addWidget(m_rescan);
    actionRow->addWidget(m_errorsLog);
    actionRow->addStretch(1);

    auto* drivesTitle = new QLabel(this);
    Theme::setClass(drivesTitle, "h2");
    drivesTitle->setObjectName("dashDrivesTitle");

    m_drives = new QTableWidget(this);
    m_drives->setColumnCount(5);
    m_drives->verticalHeader()->setVisible(false);
    m_drives->verticalHeader()->setDefaultSectionSize(24);
    m_drives->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_drives->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_drives->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_drives->setAlternatingRowColors(true);

    auto* shortcutsRow = new QHBoxLayout;
    shortcutsRow->setSpacing(8);
    auto* dupBtn = new QPushButton(this);
    dupBtn->setObjectName("shortcutDuplicates");
    auto* bigBtn = new QPushButton(this);
    bigBtn->setObjectName("shortcutBigFiles");
    auto* histBtn = new QPushButton(this);
    histBtn->setObjectName("shortcutHistory");
    shortcutsRow->addWidget(dupBtn);
    shortcutsRow->addWidget(bigBtn);
    shortcutsRow->addWidget(histBtn);
    shortcutsRow->addStretch(1);
    connect(dupBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(1); });
    connect(bigBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(2); });
    connect(histBtn, &QPushButton::clicked, this, [this]() { emit navigateTo(3); });

    auto* recentTitle = new QLabel(this);
    Theme::setClass(recentTitle, "h2");
    recentTitle->setObjectName("dashRecentTitle");

    m_recent = new QListWidget(this);
    m_recent->setMaximumHeight(150);
    Theme::setClass(m_recent, "recent");

    layout->addWidget(title);
    layout->addLayout(statsRow);
    layout->addWidget(m_lastScan);
    layout->addLayout(actionRow);
    layout->addWidget(drivesTitle);
    layout->addWidget(m_drives, 1);
    layout->addWidget(recentTitle);
    layout->addWidget(m_recent);
    layout->addLayout(shortcutsRow);
}

void DashboardPage::retranslate() {
    findChild<QLabel*>("dashTitle")->setText(i18n("nav_dashboard"));
    findChild<QLabel*>("statFilesLabel")->setText(i18n("dash_files"));
    findChild<QLabel*>("statGroupsLabel")->setText(i18n("dash_groups"));
    findChild<QLabel*>("statRecoverableLabel")->setText(i18n("dash_recoverable"));
    findChild<QLabel*>("dashDrivesTitle")->setText(i18n("dash_drives"));
    findChild<QLabel*>("dashRecentTitle")->setText(i18n("dash_recent"));
    m_rescan->setText(i18n("btn_rescan"));
    m_errorsLog->setText(i18n("dash_errors_log"));
    findChild<QPushButton*>("shortcutDuplicates")->setText(i18n("nav_duplicates"));
    findChild<QPushButton*>("shortcutBigFiles")->setText(i18n("nav_bigfiles"));
    findChild<QPushButton*>("shortcutHistory")->setText(i18n("nav_history"));
    m_drives->setHorizontalHeaderLabels({i18n("dash_drive"), i18n("dash_fs"),
                                         i18n("dash_total"), i18n("dash_free"), i18n("dash_use")});
}

void DashboardPage::refreshDrives() {
    const QVector<DriveInfo> drives = DrivesInfo::mounted();
    m_drives->setRowCount(drives.size());
    for (int i = 0; i < drives.size(); ++i) {
        const DriveInfo& d = drives[i];
        auto* pathItem = new QTableWidgetItem(d.path);
        pathItem->setToolTip(d.path);
        m_drives->setItem(i, 0, pathItem);
        m_drives->setItem(i, 1, new QTableWidgetItem(d.fileSystem));
        auto* totalItem = new QTableWidgetItem(I18n::formatBytes(d.totalBytes));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_drives->setItem(i, 2, totalItem);
        auto* freeItem = new QTableWidgetItem(I18n::formatBytes(d.freeBytes));
        freeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_drives->setItem(i, 3, freeItem);
        const int pct = d.totalBytes > 0
            ? int(100 - (double(d.freeBytes) / double(d.totalBytes)) * 100.0)
            : 0;
        auto* useItem = new QTableWidgetItem(QString::number(pct) + QStringLiteral(" %"));
        useItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_drives->setItem(i, 4, useItem);
    }
}

void DashboardPage::applyScanState(const ScanStats& stats, const QDateTime& finishedAt) {
    findChild<QLabel*>("statFiles")->setText(QString::number(stats.filesIndexed));
    findChild<QLabel*>("statGroups")->setText(QString::number(stats.duplicateGroups));
    findChild<QLabel*>("statRecoverable")->setText(I18n::formatBytes(stats.duplicateSpace));

    QString line = i18n("dash_last_scan") + QStringLiteral(": ");
    if (finishedAt.isValid()) {
        line += finishedAt.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"));
        line += QStringLiteral(" · ") + i18n("fmt_duration")
                    .arg(int(stats.durationMs / 60000))
                    .arg(int((stats.durationMs % 60000) / 1000));
    } else {
        line += i18n("dash_never");
    }
    if (stats.errors > 0) {
        line += QStringLiteral(" · ") + i18n("dash_errors_line").arg(stats.errors);
        m_errorsLog->setVisible(true);
        m_errorsLog->setToolTip(i18n("dash_errors_log_tooltip").arg(stats.errors));
    } else {
        m_errorsLog->setVisible(false);
    }
    m_lastScan->setText(line);
}

void DashboardPage::setRecentHistory(const QVector<HistoryEntry>& entries) {
    m_recent->clear();
    const int count = qMin(entries.size(), 5);
    for (int i = 0; i < count; ++i) {
        const HistoryEntry& e = entries[entries.size() - 1 - i];
        const QDateTime when = QDateTime::fromMSecsSinceEpoch(e.timestampMs);
        const QString type = e.type == QStringLiteral("scan") ? i18n("hist_type_scan")
                             : e.type == QStringLiteral("cleanup") ? i18n("hist_type_cleanup")
                                                                   : i18n("hist_type_scan_big");
        m_recent->addItem(when.toString(QStringLiteral("dd/MM HH:mm")) + QStringLiteral(" · ")
                              + type + QStringLiteral(" · ")
                              + i18n("fmt_files").arg(e.filesFound)
                              + (e.trashedCount > 0
                                     ? QStringLiteral(" · ") + i18n("hist_col_trashed")
                                           + QStringLiteral(": ") + QString::number(e.trashedCount)
                                     : QString()));
    }
}
