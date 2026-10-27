#include "BigFilesPage.h"

#include "I18n.h"
#include "PlatformHelper.h"
#include "ReviewDialog.h"
#include "Settings.h"
#include "Theme.h"

#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {

constexpr int RolePath = Qt::UserRole;
constexpr int RoleSize = Qt::UserRole + 1;
constexpr int RoleMtime = Qt::UserRole + 2;
constexpr int RoleMini = Qt::UserRole + 3;

class SizeItem : public QTableWidgetItem {
public:
    explicit SizeItem(qint64 bytes) : QTableWidgetItem(), m_bytes(bytes) {
        setText(I18n::formatBytes(bytes));
        setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    bool operator<(const QTableWidgetItem& other) const override {
        return m_bytes < other.data(Qt::UserRole + 1).toLongLong();
    }
    qint64 bytes() const { return m_bytes; }

private:
    qint64 m_bytes;
};

class TimeItem : public QTableWidgetItem {
public:
    explicit TimeItem(qint64 secs) : m_secs(secs) {
        setText(QDateTime::fromSecsSinceEpoch(secs).toString(QStringLiteral("dd/MM/yyyy HH:mm")));
    }
    bool operator<(const QTableWidgetItem& other) const override {
        return m_secs < other.data(Qt::UserRole + 2).toLongLong();
    }

private:
    qint64 m_secs;
};

}

BigFilesPage::BigFilesPage(QWidget* parent) : QWidget(parent) {
    buildUi();
    retranslate();
}

void BigFilesPage::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    auto* title = new QLabel(this);
    Theme::setClass(title, "h1");
    title->setObjectName("bigTitle");

    auto* controls = new QHBoxLayout;
    controls->setSpacing(8);
    controls->addWidget(new QLabel(i18n("big_threshold"), this));
    m_threshold = new QComboBox(this);
    m_customSize = new QSpinBox(this);
    m_customSize->setRange(1, 1024 * 100);
    m_customSize->setSuffix(" MB");
    m_customSize->setValue(250);
    m_customSize->hide();
    controls->addWidget(m_threshold);
    controls->addWidget(m_customSize);
    m_startBtn = new QPushButton(this);
    Theme::setClass(m_startBtn, "primary");
    m_pauseBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);
    m_pauseBtn->hide();
    m_cancelBtn->hide();
    controls->addWidget(m_startBtn);
    controls->addWidget(m_pauseBtn);
    controls->addWidget(m_cancelBtn);
    controls->addStretch(1);
    connect(m_threshold, &QComboBox::currentIndexChanged, this, [this](int idx) {
        m_customSize->setVisible(idx == m_threshold->count() - 1);
    });

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(8);
    m_search = new QLineEdit(this);
    m_extFilter = new QComboBox(this);
    m_extFilter->setFixedWidth(160);
    filterRow->addWidget(m_search, 1);
    filterRow->addWidget(m_extFilter, 0);
    connect(m_search, &QLineEdit::textChanged, this, [this]() { applyFilters(); });
    connect(m_extFilter, &QComboBox::currentIndexChanged, this, [this]() { applyFilters(); });

    m_table = new QTableWidget(this);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(26);
    m_table->setSortingEnabled(true);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, &BigFilesPage::showContextMenu);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() { updateSelectedLabel(); });
    connect(m_table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem* it) {
        PlatformHelper::openFile(m_table->item(it->row(), 0)->data(RolePath).toString());
    });

    m_progress = new QProgressBar(this);
    m_progress->setTextVisible(false);
    m_progress->setFixedHeight(8);
    m_progress->setRange(0, 0);
    m_progress->hide();

    m_status = new QLabel(this);
    Theme::setClass(m_status, "muted");

    m_selected = new QLabel(this);
    Theme::setClass(m_selected, "muted");

    auto* bottom = new QHBoxLayout;
    bottom->setSpacing(8);
    m_reviewBtn = new QPushButton(this);
    Theme::setClass(m_reviewBtn, "danger");
    bottom->addWidget(m_selected);
    bottom->addStretch(1);
    bottom->addWidget(m_reviewBtn);

    layout->addWidget(title);
    layout->addLayout(controls);
    layout->addLayout(filterRow);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_progress);
    layout->addWidget(m_status);
    layout->addLayout(bottom);

    connect(m_startBtn, &QPushButton::clicked, this, &BigFilesPage::startScan);
    connect(m_reviewBtn, &QPushButton::clicked, this, &BigFilesPage::reviewSelection);
}

void BigFilesPage::retranslateTable() {
    m_table->setSortingEnabled(false);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({i18n("big_col_name"), i18n("big_col_size"),
                                        i18n("big_col_mtime"), i18n("big_col_ext")});
    m_table->setSortingEnabled(true);
}

void BigFilesPage::retranslate() {
    findChild<QLabel*>("bigTitle")->setText(i18n("nav_bigfiles"));
    m_startBtn->setText(i18n("btn_scan_big"));
    m_pauseBtn->setText(i18n("btn_pause"));
    m_cancelBtn->setText(i18n("btn_cancel_scan"));
    m_reviewBtn->setText(i18n("btn_review"));
    m_search->setPlaceholderText(i18n("big_search"));
    const int idx = m_threshold->currentIndex();
    m_threshold->blockSignals(true);
    m_threshold->clear();
    m_threshold->addItem(QStringLiteral("100 MB"), 100LL * 1024 * 1024);
    m_threshold->addItem(QStringLiteral("500 MB"), 500LL * 1024 * 1024);
    m_threshold->addItem(QStringLiteral("1 GB"), 1024LL * 1024 * 1024);
    m_threshold->addItem(i18n("big_custom"), -1);
    m_threshold->setCurrentIndex(idx < 0 ? 0 : idx);
    m_threshold->blockSignals(false);
    retranslateTable();
    updateSelectedLabel();
}

void BigFilesPage::startScan() {
    emit scanRequested(buildConfig());
}

ScanConfig BigFilesPage::buildConfig() const {
    ScanConfig config;
    config.mode = ScanMode::BigFiles;
    config.roots = {QDir::homePath()};
    config.ignoredDirs = Settings::instance().ignoredDirs();
    config.ignoredExtensions = Settings::instance().ignoredExtensions();
    const qint64 data = m_threshold->currentData().toLongLong();
    if (data < 0) {
        config.minFileSize = qint64(m_customSize->value()) * 1024 * 1024;
    } else {
        config.minFileSize = data;
    }
    config.maxConcurrent = Settings::instance().maxConcurrent();
    return config;
}

void BigFilesPage::setScanBusy(bool busy) {
    m_busy = busy;
    m_startBtn->setVisible(!busy);
    m_pauseBtn->setVisible(busy);
    m_cancelBtn->setVisible(busy);
    m_progress->setVisible(busy);
    if (busy) {
        m_records.clear();
        m_table->setRowCount(0);
        m_extFilter->clear();
    }
    updateSelectedLabel();
}

void BigFilesPage::setPaused(bool paused) {
    m_pauseBtn->setText(paused ? i18n("btn_resume") : i18n("btn_pause"));
}

void BigFilesPage::applyRecords(const QVector<FileRecord>& records, const ScanStats& stats) {
    m_records = records;
    m_extFilter->blockSignals(true);
    m_extFilter->clear();
    m_extFilter->addItem(i18n("big_ext_all"));
    QSet<QString> exts;
    for (const FileRecord& rec : records) {
        const QString ext = QFileInfo(rec.path).suffix().toLower();
        if (!ext.isEmpty()) exts.insert(ext);
    }
    QStringList sorted = exts.values();
    std::sort(sorted.begin(), sorted.end());
    for (const QString& ext : sorted) m_extFilter->addItem("." + ext);
    m_extFilter->blockSignals(false);

    applyFilters();

    m_status->setText(i18n("state_done") + QStringLiteral(" Â· ")
                          + i18n("fmt_files").arg(stats.filesIndexed) + QStringLiteral(" Â· ")
                          + I18n::formatBytes(stats.bytesIndexed)
                          + (stats.errors > 0
                                 ? QStringLiteral(" Â· ") + i18n("dash_errors_line").arg(stats.errors)
                                 : QString()));
}

void BigFilesPage::applyFilters() {
    m_table->setSortingEnabled(false);
    retranslateTable();
    m_table->setRowCount(0);

    const QString needle = m_search->text().trimmed();
    const QString extSel = m_extFilter->currentText();

    qint64 shown = 0;
    m_totalBytes = 0;
    for (const FileRecord& rec : m_records) {
        const QString name = QFileInfo(rec.path).fileName();
        const QString ext = QFileInfo(rec.path).suffix().toLower();
        if (!needle.isEmpty() && !name.contains(needle, Qt::CaseInsensitive)) continue;
        if (m_extFilter->currentIndex() > 0 && ("." + ext) != extSel) continue;

        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* nameItem = new QTableWidgetItem(name);
        nameItem->setData(RolePath, rec.path);
        nameItem->setData(RoleSize, rec.size);
        nameItem->setData(RoleMtime, rec.mtimeSec);
        nameItem->setToolTip(rec.path);
        m_table->setItem(row, 0, nameItem);

        auto* sizeItem = new SizeItem(rec.size);
        sizeItem->setData(RoleSize, rec.size);
        m_table->setItem(row, 1, sizeItem);

        auto* timeItem = new TimeItem(rec.mtimeSec);
        timeItem->setData(RoleMtime, rec.mtimeSec);
        m_table->setItem(row, 2, timeItem);

        m_table->setItem(row, 3, new QTableWidgetItem(ext));

        shown++;
        m_totalBytes += rec.size;
    }
    m_table->setSortingEnabled(true);
    m_status->setText(i18n("fmt_rows").arg(shown) + QStringLiteral(" Â· ")
                          + I18n::formatBytes(m_totalBytes));
    updateSelectedLabel();
}

void BigFilesPage::updateSelectedLabel() {
    const auto selected = m_table->selectedItems();
    qint64 bytes = 0;
    int rows = 0;
    QSet<int> seenRows;
    for (QTableWidgetItem* it : selected) {
        if (!seenRows.contains(it->row())) {
            seenRows.insert(it->row());
            rows++;
            const QTableWidgetItem* nameItem = m_table->item(it->row(), 0);
            if (nameItem) bytes += nameItem->data(RoleSize).toLongLong();
        }
    }
    m_selected->setText(i18n("big_selected").arg(rows).arg(I18n::formatBytes(bytes)));
    m_reviewBtn->setEnabled(rows > 0 && !m_busy);
}

void BigFilesPage::reviewSelection() {
    const auto selected = m_table->selectedItems();
    QSet<int> seenRows;
    QVector<CleanupItem> items;
    for (QTableWidgetItem* it : selected) {
        if (!seenRows.contains(it->row())) {
            seenRows.insert(it->row());
            const QTableWidgetItem* nameItem = m_table->item(it->row(), 0);
            if (!nameItem) continue;
            CleanupItem item;
            item.path = nameItem->data(RolePath).toString();
            item.size = nameItem->data(RoleSize).toLongLong();
            item.mtimeSec = nameItem->data(RoleMtime).toLongLong();
            item.miniHash = QByteArray();
            items.append(item);
        }
    }
    if (items.isEmpty()) {
        QMessageBox::information(this, i18n("review_title"), i18n("review_empty"));
        return;
    }
    ReviewDialog dialog(items, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit cleanupRequested(items);
    }
}

void BigFilesPage::showContextMenu(const QPoint& pos) {
    const QTableWidgetItem* nameItem = m_table->itemAt(pos);
    if (!nameItem) return;
    QMenu menu(this);
    QAction* openAct = menu.addAction(i18n("btn_open_file"));
    QAction* revealAct = menu.addAction(i18n("btn_open_location"));
    QAction* chosen = menu.exec(m_table->viewport()->mapToGlobal(pos));
    const QString path = m_table->item(nameItem->row(), 0)->data(RolePath).toString();
    if (chosen == openAct) {
        PlatformHelper::openFile(path);
    } else if (chosen == revealAct) {
        PlatformHelper::revealInShell(path);
    }
}
