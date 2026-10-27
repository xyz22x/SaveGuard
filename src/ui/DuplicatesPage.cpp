#include "DuplicatesPage.h"

#include "I18n.h"
#include "PlatformHelper.h"
#include "ReviewDialog.h"
#include "Settings.h"
#include "Theme.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSet>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

constexpr int RolePath = Qt::UserRole;
constexpr int RoleSize = Qt::UserRole + 1;
constexpr int RoleMtime = Qt::UserRole + 2;
constexpr int RoleMini = Qt::UserRole + 3;

CleanupItem itemFromTreeItem(QTreeWidgetItem* it) {
    CleanupItem item;
    item.path = it->data(0, RolePath).toString();
    item.size = it->data(0, RoleSize).toLongLong();
    item.mtimeSec = it->data(0, RoleMtime).toLongLong();
    item.miniHash = QByteArray::fromBase64(it->data(0, RoleMini).toString().toLatin1());
    return item;
}

}

DuplicatesPage::DuplicatesPage(QWidget* parent) : QWidget(parent) {
    buildUi();
    retranslate();
}

void DuplicatesPage::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    auto* title = new QLabel(this);
    Theme::setClass(title, "h1");
    title->setObjectName("dupTitle");

    auto* hint = new QLabel(this);
    Theme::setClass(hint, "muted");
    hint->setObjectName("dupHint");
    hint->setWordWrap(true);

    auto* foldersRow = new QHBoxLayout;
    foldersRow->setSpacing(10);

    m_folders = new QListWidget(this);
    m_folders->setFixedWidth(300);
    m_folders->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* folderButtons = new QVBoxLayout;
    folderButtons->setSpacing(8);
    auto* addBtn = new QPushButton(this);
    addBtn->setObjectName("dupAdd");
    auto* rmBtn = new QPushButton(this);
    rmBtn->setObjectName("dupRm");
    folderButtons->addWidget(addBtn);
    folderButtons->addWidget(rmBtn);
    folderButtons->addStretch(1);

    foldersRow->addWidget(m_folders, 0);
    foldersRow->addLayout(folderButtons, 0);

    m_tree = new QTreeWidget(this);
    m_tree->setAlternatingRowColors(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(m_folders, &QListWidget::itemSelectionChanged, this, [this, rmBtn]() {
        rmBtn->setEnabled(!m_folders->selectedItems().isEmpty());
    });
    connect(addBtn, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, i18n("btn_add_folder"), QDir::homePath());
        if (dir.isEmpty()) return;
        const QString norm = QDir::fromNativeSeparators(dir);
        for (int i = 0; i < m_folders->count(); ++i) {
            if (m_folders->item(i)->text().compare(norm, Qt::CaseInsensitive) == 0) return;
        }
        m_folders->addItem(norm);
    });
    connect(rmBtn, &QPushButton::clicked, this, [this]() {
        const auto selected = m_folders->selectedItems();
        for (QListWidgetItem* it : selected) delete it;
    });
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &DuplicatesPage::showContextMenu);
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* it, int) {
        if (!it->parent()) return;
        PlatformHelper::openFile(it->data(0, RolePath).toString());
    });

    m_progress = new QProgressBar(this);
    m_progress->setTextVisible(false);
    m_progress->setFixedHeight(8);
    m_progress->setRange(0, 0);
    m_progress->hide();

    m_status = new QLabel(this);
    Theme::setClass(m_status, "muted");

    m_summary = new QLabel(this);
    Theme::setClass(m_summary, "muted");

    auto* controls = new QHBoxLayout;
    controls->setSpacing(8);
    m_startBtn = new QPushButton(this);
    Theme::setClass(m_startBtn, "primary");
    m_pauseBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);
    m_reviewBtn = new QPushButton(this);
    Theme::setClass(m_reviewBtn, "danger");
    m_pauseBtn->hide();
    m_cancelBtn->hide();
    controls->addWidget(m_startBtn);
    controls->addWidget(m_pauseBtn);
    controls->addWidget(m_cancelBtn);
    controls->addStretch(1);
    controls->addWidget(m_reviewBtn);

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addLayout(foldersRow);
    layout->addWidget(m_tree, 1);
    layout->addWidget(m_progress);
    layout->addWidget(m_status);
    layout->addLayout(controls);
    layout->addWidget(m_summary);

    connect(m_startBtn, &QPushButton::clicked, this, &DuplicatesPage::startScan);
    connect(m_reviewBtn, &QPushButton::clicked, this, &DuplicatesPage::reviewSelection);
}

void DuplicatesPage::retranslate() {
    findChild<QLabel*>("dupTitle")->setText(i18n("nav_duplicates"));
    findChild<QLabel*>("dupHint")->setText(i18n("dup_folders_hint"));
    findChild<QPushButton*>("dupAdd")->setText(i18n("btn_add_folder"));
    findChild<QPushButton*>("dupRm")->setText(i18n("btn_remove_folder"));
    m_startBtn->setText(i18n("btn_start_scan"));
    m_pauseBtn->setText(i18n("btn_pause"));
    m_cancelBtn->setText(i18n("btn_cancel_scan"));
    m_reviewBtn->setText(i18n("btn_review"));
    retranslateTree();
    updateSummary();
}

void DuplicatesPage::retranslateTree() {
    m_tree->setHeaderLabels({i18n("dup_col_file"), i18n("dup_col_size"),
                             i18n("dup_col_mtime"), i18n("dup_col_action")});
}

void DuplicatesPage::updateSummary() {
    qint64 selectedBytes = 0;
    int selectedCount = 0;
    for (int g = 0; g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem* group = m_tree->topLevelItem(g);
        for (int c = 0; c < group->childCount(); ++c) {
            QTreeWidgetItem* child = group->child(c);
            if (child->checkState(3) == Qt::Checked) {
                selectedCount++;
                selectedBytes += child->data(0, RoleSize).toLongLong();
            }
        }
    }
    if (m_groupCount == 0 && m_tree->topLevelItemCount() == 0) {
        m_summary->setText(m_busy ? i18n("dup_running") : i18n("dup_empty"));
    } else if (m_tree->topLevelItemCount() == 0) {
        m_summary->setText(i18n("dup_none_found"));
    } else {
        m_summary->setText(i18n("dup_summary_groups")
                               .arg(m_groupCount)
                               .arg(selectedCount)
                               .arg(I18n::formatBytes(selectedBytes)));
    }
    m_reviewBtn->setEnabled(selectedCount > 0 && !m_busy);
}

void DuplicatesPage::startScan() {
    if (m_folders->count() == 0) {
        QMessageBox::information(this, i18n("app_name"), i18n("err_no_folder"));
        return;
    }
    QStringList roots;
    for (int i = 0; i < m_folders->count(); ++i) roots << m_folders->item(i)->text();
    for (const QString& root : roots) {
        QFileInfo info(root);
        if (!info.exists() || !info.isDir()) {
            QMessageBox::warning(this, i18n("app_name"), i18n("err_root_missing").arg(root));
            return;
        }
    }
    emit scanRequested(buildConfig());
}

ScanConfig DuplicatesPage::buildConfig() const {
    ScanConfig config;
    config.mode = ScanMode::Duplicates;
    for (int i = 0; i < m_folders->count(); ++i) config.roots << m_folders->item(i)->text();
    config.ignoredDirs = Settings::instance().ignoredDirs();
    config.ignoredExtensions = Settings::instance().ignoredExtensions();
    config.minFileSize = Settings::instance().minFileSize();
    config.maxConcurrent = Settings::instance().maxConcurrent();
    return config;
}

void DuplicatesPage::applyConfig(const ScanConfig& config) {
    m_folders->clear();
    for (const QString& root : config.roots) m_folders->addItem(root);
}

void DuplicatesPage::setScanBusy(bool busy) {
    m_busy = busy;
    m_startBtn->setVisible(!busy);
    m_pauseBtn->setVisible(busy);
    m_cancelBtn->setVisible(busy);
    m_progress->setVisible(busy);
    if (busy) {
        m_tree->clear();
        m_groupCount = 0;
    }
    updateSummary();
}

void DuplicatesPage::setPaused(bool paused) {
    m_pauseBtn->setText(paused ? i18n("btn_resume") : i18n("btn_pause"));
}

void DuplicatesPage::applyGroups(const QVector<DuplicateGroup>& groups, const ScanStats& stats) {
    m_tree->clear();
    m_groupCount = groups.size();
    retranslateTree();

    for (const DuplicateGroup& g : groups) {
        auto* groupItem = new QTreeWidgetItem(m_tree);
        groupItem->setText(0, i18n("dup_group_label")
                                  .arg(g.files.size())
                                  .arg(I18n::formatBytes(g.recoverable())));
        groupItem->setFirstColumnSpanned(true);
        for (int i = 0; i < g.files.size(); ++i) {
            const FileRecord& rec = g.files[i];
            auto* child = new QTreeWidgetItem(groupItem);
            child->setText(0, rec.path);
            child->setToolTip(0, rec.path);
            child->setText(1, I18n::formatBytes(rec.size));
            child->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
            child->setText(2, QDateTime::fromSecsSinceEpoch(rec.mtimeSec)
                                  .toString(QStringLiteral("dd/MM/yyyy HH:mm")));
            child->setCheckState(3, i == 0 ? Qt::Unchecked : Qt::Checked);
            child->setData(0, RolePath, rec.path);
            child->setData(0, RoleSize, rec.size);
            child->setData(0, RoleMtime, rec.mtimeSec);
            child->setData(0, RoleMini, QString::fromLatin1(rec.miniHash.toBase64()));
        }
        groupItem->setExpanded(true);
    }

    m_status->setText(i18n("state_done") + QStringLiteral(" · ")
                          + i18n("fmt_files").arg(stats.filesIndexed) + QStringLiteral(" · ")
                          + I18n::formatBytes(stats.bytesIndexed)
                          + (stats.errors > 0
                                 ? QStringLiteral(" · ") + i18n("dash_errors_line").arg(stats.errors)
                                 : QString()));
    updateSummary();
}

void DuplicatesPage::removePathsFromResults(const QSet<QString>& paths) {
    for (int g = m_tree->topLevelItemCount() - 1; g >= 0; --g) {
        QTreeWidgetItem* group = m_tree->topLevelItem(g);
        for (int c = group->childCount() - 1; c >= 0; --c) {
            QTreeWidgetItem* child = group->child(c);
            if (paths.contains(child->data(0, RolePath).toString())) {
                delete child;
            } else if (child->checkState(3) == Qt::Checked) {
                child->setCheckState(3, Qt::Unchecked);
            }
        }
        if (group->childCount() < 2) delete group;
    }
    m_groupCount = m_tree->topLevelItemCount();
    updateSummary();
}

void DuplicatesPage::clearResults() {
    m_tree->clear();
    m_groupCount = 0;
    m_status->clear();
    updateSummary();
}

QVector<CleanupItem> DuplicatesPage::selectedItems(bool* ok, QString* error) const {
    QVector<CleanupItem> items;
    for (int g = 0; g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem* group = m_tree->topLevelItem(g);
        int checked = 0;
        for (int c = 0; c < group->childCount(); ++c) {
            QTreeWidgetItem* child = group->child(c);
            if (child->checkState(3) == Qt::Checked) {
                checked++;
                items.append(itemFromTreeItem(child));
            }
        }
        if (checked == group->childCount() && group->childCount() > 0) {
            if (ok) *ok = false;
            if (error) *error = i18n("dup_at_least_one");
            return {};
        }
    }
    if (ok) *ok = true;
    return items;
}

void DuplicatesPage::reviewSelection() {
    bool ok = false;
    QString error;
    const QVector<CleanupItem> items = selectedItems(&ok, &error);
    if (!ok) {
        QMessageBox::information(this, i18n("review_title"), error);
        return;
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

void DuplicatesPage::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = m_tree->itemAt(pos);
    if (!item || !item->parent()) return;
    QMenu menu(this);
    QAction* openAct = menu.addAction(i18n("btn_open_file"));
    QAction* revealAct = menu.addAction(i18n("btn_open_location"));
    menu.addSeparator();
    QAction* keepAct = menu.addAction(i18n("dup_ctx_keep"));
    QAction* removeAct = menu.addAction(i18n("dup_ctx_remove"));
    QAction* chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
    const QString path = item->data(0, RolePath).toString();
    if (chosen == openAct) {
        PlatformHelper::openFile(path);
    } else if (chosen == revealAct) {
        PlatformHelper::revealInShell(path);
    } else if (chosen == keepAct) {
        item->setCheckState(3, Qt::Unchecked);
    } else if (chosen == removeAct) {
        item->setCheckState(3, Qt::Checked);
    }
    updateSummary();
}
