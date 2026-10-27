#include "MainWindow.h"

#include "CleanupManager.h"
#include "DashboardPage.h"
#include "DuplicatesPage.h"
#include "BigFilesPage.h"
#include "HistoryPage.h"
#include "HistoryStore.h"
#include "I18n.h"
#include "Scanner.h"
#include "Settings.h"
#include "SettingsPage.h"
#include "Theme.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressBar>
#include <QSet>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStringList>
#include <QStyle>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow() {
    m_history = new HistoryStore(HistoryStore::defaultFilePath());

    m_scanThread = new QThread(this);
    m_scanner = new Scanner();
    m_scanner->moveToThread(m_scanThread);
    m_scanThread->start();

    m_cleanupThread = new QThread(this);
    m_cleaner = new CleanupManager();
    m_cleaner->moveToThread(m_cleanupThread);
    m_cleanupThread->start();

    buildUi();
    connectPages();
    retranslateAll();
    refreshHistory();
    m_dashboard->refreshDrives();

    setMinimumSize(960, 620);
    resize(1120, 720);
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* sidebarWrap = new QWidget(this);
    sidebarWrap->setFixedWidth(210);
    auto* side = new QVBoxLayout(sidebarWrap);
    side->setContentsMargins(14, 18, 10, 14);
    side->setSpacing(10);

    auto* brand = new QLabel(i18n("app_name"), this);
    Theme::setClass(brand, "brand");
    auto* brandSub = new QLabel(i18n("tagline"), this);
    Theme::setClass(brandSub, "brandSub");
    brandSub->setWordWrap(true);

    m_nav = new QListWidget(this);
    Theme::setClass(m_nav, "sidebar");
    m_nav->setSelectionMode(QAbstractItemView::SingleSelection);
    m_nav->setIconSize(QSize(18, 18));
    auto* dashItem = new QListWidgetItem(QIcon(":/icons/dashboard.svg"), i18n("nav_dashboard"));
    auto* dupItem = new QListWidgetItem(QIcon(":/icons/duplicates.svg"), i18n("nav_duplicates"));
    auto* bigItem = new QListWidgetItem(QIcon(":/icons/bigfiles.svg"), i18n("nav_bigfiles"));
    auto* histItem = new QListWidgetItem(QIcon(":/icons/history.svg"), i18n("nav_history"));
    auto* setItem = new QListWidgetItem(QIcon(":/icons/settings.svg"), i18n("nav_settings"));
    m_nav->addItem(dashItem);
    m_nav->addItem(dupItem);
    m_nav->addItem(bigItem);
    m_nav->addItem(histItem);
    m_nav->addItem(setItem);

    side->addWidget(brand);
    side->addWidget(brandSub);
    side->addSpacing(8);
    side->addWidget(m_nav);
    side->addStretch(1);

    auto* right = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    auto* topbar = new QWidget(this);
    topbar->setObjectName("topbar");
    topbar->setAutoFillBackground(true);
    auto* topLayout = new QHBoxLayout(topbar);
    topLayout->setContentsMargins(24, 12, 24, 12);
    topLayout->setSpacing(12);

    m_topTitle = new QLabel(this);
    Theme::setClass(m_topTitle, "h2");
    m_topStatus = new QLabel(this);
    Theme::setClass(m_topStatus, "muted");
    m_pauseBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);
    m_pauseBtn->hide();
    m_cancelBtn->hide();
    topLayout->addWidget(m_topTitle);
    topLayout->addWidget(m_topStatus, 1);
    topLayout->addWidget(m_pauseBtn);
    topLayout->addWidget(m_cancelBtn);

    m_pages = new QStackedWidget(this);
    m_dashboard = new DashboardPage(this);
    m_duplicates = new DuplicatesPage(this);
    m_bigFiles = new BigFilesPage(this);
    m_historyPage = new HistoryPage(this);
    m_settingsPage = new SettingsPage(this);
    m_pages->addWidget(m_dashboard);
    m_pages->addWidget(m_duplicates);
    m_pages->addWidget(m_bigFiles);
    m_pages->addWidget(m_historyPage);
    m_pages->addWidget(m_settingsPage);

    rightLayout->addWidget(topbar);
    rightLayout->addWidget(m_pages, 1);

    root->addWidget(sidebarWrap);
    root->addWidget(right, 1);

    auto* line = new QFrame(this);
    line->setFixedWidth(1);
    Theme::setClass(line, "line");
    root->addWidget(line);

    setCentralWidget(central);

    connect(m_nav, &QListWidget::currentRowChanged, this, &MainWindow::switchPage);
    m_nav->setCurrentRow(0);

    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseOrResume);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelScan);
}

void MainWindow::connectPages() {
    connect(m_duplicates, &DuplicatesPage::scanRequested, this, &MainWindow::onScanRequested);
    connect(m_duplicates, &DuplicatesPage::cleanupRequested, this, &MainWindow::onCleanupRequested);
    connect(m_bigFiles, &BigFilesPage::scanRequested, this, &MainWindow::onBigScanRequested);
    connect(m_bigFiles, &BigFilesPage::cleanupRequested, this, &MainWindow::onCleanupRequested);
    connect(m_dashboard, &DashboardPage::navigateTo, this, &MainWindow::switchPage);
    connect(m_dashboard, &DashboardPage::rescanRequested, this, [this]() {
        switchPage(1);
    });
    connect(m_dashboard, &DashboardPage::openErrorsLogRequested, this, [this]() {
        if (m_lastErrorLog.isEmpty()) return;
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_lastErrorLog));
    });
    connect(m_settingsPage, &SettingsPage::settingsSaved, this, [this]() {
        m_dashboard->refreshDrives();
        refreshHistory();
    });

    connect(m_scanner, &Scanner::stateChanged, this, &MainWindow::onScanStateChanged, Qt::QueuedConnection);
    connect(m_scanner, &Scanner::batch, this, &MainWindow::onScanBatch, Qt::QueuedConnection);
    connect(m_scanner, &Scanner::finishedStats, this, &MainWindow::onScanFinished, Qt::QueuedConnection);
    connect(m_scanner, &Scanner::duplicatesReady, this, &MainWindow::onDuplicatesReady, Qt::QueuedConnection);

    connect(m_cleaner, &CleanupManager::itemResult, this, &MainWindow::onCleanupItem, Qt::QueuedConnection);
    connect(m_cleaner, &CleanupManager::finishedCleanup, this, &MainWindow::onCleanupFinished, Qt::QueuedConnection);
}

void MainWindow::retranslateAll() {
    static const char* keys[] = {"nav_dashboard", "nav_duplicates", "nav_bigfiles",
                                 "nav_history", "nav_settings"};
    for (int i = 0; i < m_nav->count(); ++i) {
        m_nav->item(i)->setText(I18n::instance().t(QString::fromLatin1(keys[i])));
    }
    m_dashboard->retranslate();
    m_duplicates->retranslate();
    m_bigFiles->retranslate();
    m_historyPage->retranslate();
    m_settingsPage->retranslate();
    m_pauseBtn->setText(i18n("btn_pause"));
    m_cancelBtn->setText(i18n("btn_cancel_scan"));
    m_dashboard->refreshDrives();
    onPageChanged(m_pages->currentIndex());
}

void MainWindow::switchPage(int index) {
    if (index < 0 || index >= m_pages->count()) return;
    m_pages->setCurrentIndex(index);
    onPageChanged(index);
}

void MainWindow::onPageChanged(int index) {
    static const char* keys[] = {"nav_dashboard", "nav_duplicates", "nav_bigfiles",
                                 "nav_history", "nav_settings"};
    if (index >= 0 && index < 5) {
        m_topTitle->setText(I18n::instance().t(QString::fromLatin1(keys[index])));
    }
    if (index == 0) {
        m_dashboard->refreshDrives();
        m_dashboard->setRecentHistory(m_history->entries());
        m_dashboard->applyScanState(m_lastStats, m_lastScanFinishedAt);
    }
    if (index == 3) refreshHistory();
}

void MainWindow::updateTopState(const QString& stateKey) {
    m_topStatus->setText(i18n("app_name") + QStringLiteral(" · ")
                             + I18n::instance().t(stateKey));
}

void MainWindow::onScanRequested(const ScanConfig& config) {
    startScan(config);
}

void MainWindow::onBigScanRequested(const ScanConfig& config) {
    startScan(config);
}

void MainWindow::startScan(const ScanConfig& config) {
    if (m_scanBusy || m_scanner->isBusy()) {
        QMessageBox::information(this, i18n("app_name"), i18n("scan_state_busy"));
        return;
    }
    setScanBusy(true);
    m_lastRecords.clear();
    m_lastGroups.clear();
    m_activeRoots = config.roots.join(QStringLiteral(", "));
    m_activeMode = config.mode;
    QMetaObject::invokeMethod(m_scanner, [this, config]() { m_scanner->run(config); },
                              Qt::QueuedConnection);
}

void MainWindow::setScanBusy(bool busy) {
    m_scanBusy = busy;
    m_duplicates->setScanBusy(busy);
    m_bigFiles->setScanBusy(busy);
    m_pauseBtn->setVisible(busy);
    m_cancelBtn->setVisible(busy);
    if (!busy) {
        m_pauseBtn->setText(i18n("btn_pause"));
        m_duplicates->setPaused(false);
        m_bigFiles->setPaused(false);
    }
    updateTopState(busy ? QStringLiteral("state_scanning") : QStringLiteral("state_idle"));
}

void MainWindow::onScanStateChanged(const QString& state) {
    if (state.startsWith("scan_state_hash:")) {
        const QStringList parts = state.mid(16).split(':');
        if (parts.size() == 2) {
            m_topStatus->setText(i18n("scan_state_hash").arg(parts[0]).arg(parts[1]));
            return;
        }
    }
    const QHash<QString, QString> map = {
        {"scan_state_enumerate", i18n("scan_state_enumerate")},
        {"scan_state_hash", i18n("scan_state_hash").arg(0).arg(0)},
        {"scan_state_group", i18n("scan_state_group")},
    };
    m_topStatus->setText(map.value(state, state));
}

void MainWindow::onScanBatch(const QVector<FileRecord>& records) {
    for (const FileRecord& rec : records) m_lastRecords.append(rec);
}

void MainWindow::onScanFinished(const ScanStats& stats) {
    if (stats.state == QStringLiteral("busy")) {
        setScanBusy(false);
        return;
    }
    m_lastStats = stats;
    m_lastScanFinishedAt = stats.state == QStringLiteral("done") ? QDateTime::currentDateTime() : QDateTime();
    setScanBusy(false);
    if (stats.state == QStringLiteral("canceled")) {
        m_topStatus->setText(i18n("state_canceled"));
    } else {
        m_topStatus->setText(i18n("state_done"));
    }
    if (stats.state == QStringLiteral("done") && stats.filesIndexed > 0) {
        HistoryEntry entry;
        entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
        entry.durationMs = stats.durationMs;
        entry.type = m_activeMode == ScanMode::BigFiles ? QStringLiteral("scan_big")
                                                        : QStringLiteral("scan");
        entry.roots = m_activeRoots;
        entry.filesFound = stats.filesIndexed;
        entry.recoverableBytes = stats.duplicateSpace;
        entry.errorCount = stats.errors;
        addHistoryEntry(entry);
    }
    m_dashboard->applyScanState(m_lastStats, m_lastScanFinishedAt);
    if (!m_lastGroups.isEmpty()) {
        m_duplicates->applyGroups(m_lastGroups, m_lastStats);
        m_lastGroups.clear();
    }
    writeErrorLog(stats);
}

void MainWindow::writeErrorLog(const ScanStats& stats) {
    m_lastErrorLog.clear();
    if (stats.errors <= 0) return;
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                            + QStringLiteral("/logs");
    QDir().mkpath(dir);
    const QString file = dir + QStringLiteral("/erros-")
                             + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))
                             + QStringLiteral(".log");
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "SaveGuard - registro de erros da analise\n";
    out << "Data: " << QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")) << "\n";
    out << "Pastas: " << m_activeRoots << "\n";
    out << "Tipo: " << (m_activeMode == ScanMode::BigFiles ? QStringLiteral("arquivos grandes")
                                                           : QStringLiteral("duplicatas"))
        << "\n";
    out << "Arquivos analisados: " << stats.filesIndexed << "\n";
    out << "Total de erros: " << stats.errors << "\n";
    out << "Codigos: ROOT=pasta raiz inacessivel | DENIED-DIR=pasta sem permissao de leitura | "
           "DIR-READ=leitura da pasta interrompida | REPARSE=atalho/reparse ilegivel | "
           "FILE-ACCESS=arquivo bloqueado (comum em aliases de app do WindowsApps) | "
           "FILE-TIME=data de modificacao indisponivel | HASH-FAIL=arquivo ilegivel durante o hash\n";
    for (const QString& s : stats.errorSamples) out << s << "\n";
    const qint64 extra = stats.errors - qint64(stats.errorSamples.size());
    if (extra > 0) out << "... e " << extra << " ocorrencia(s) nao listadas (limite de 200)\n";
    out.flush();
    f.close();
    m_lastErrorLog = file;
}

void MainWindow::onDuplicatesReady(const QVector<DuplicateGroup>& groups) {
    m_lastGroups = groups;
    m_dashboard->applyScanState(m_lastStats, m_lastScanFinishedAt);
}

void MainWindow::onPauseOrResume() {
    if (!m_scanBusy) return;
    const bool paused = m_pauseBtn->text() == i18n("btn_pause");
    if (paused) {
        m_scanner->pause();
        m_pauseBtn->setText(i18n("btn_resume"));
        updateTopState(QStringLiteral("state_paused"));
    } else {
        m_scanner->resume();
        m_pauseBtn->setText(i18n("btn_pause"));
        updateTopState(QStringLiteral("state_scanning"));
    }
}

void MainWindow::onCancelScan() {
    if (m_scanBusy) m_scanner->cancel();
}

void MainWindow::onCleanupRequested(const QVector<CleanupItem>& items) {
    if (m_scanBusy || m_scanner->isBusy()) {
        QMessageBox::information(this, i18n("app_name"), i18n("dup_running"));
        return;
    }
    if (m_cleanupBusy || m_cleaner->isBusy()) return;
    m_cleanupBusy = true;
    m_pendingCleanup = items;
    updateTopState(QStringLiteral("state_working"));
    QMetaObject::invokeMethod(m_cleaner, [this, items]() { m_cleaner->run(items, true); },
                              Qt::QueuedConnection);
}

void MainWindow::onCleanupItem(const CleanupResult& result) {
    if (!result.success) {
        m_cleanupFailures.append(result.path + QStringLiteral("  —  ")
                                     + I18n::instance().t(result.error));
    }
}

void MainWindow::onCleanupFinished(int success, qint64 bytesFreed, int fail) {
    m_cleanupBusy = false;
    QSet<QString> removed;
    for (const CleanupItem& item : m_pendingCleanup) removed.insert(item.path);
    m_duplicates->removePathsFromResults(removed);

    HistoryEntry entry;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
    entry.durationMs = 0;
    entry.type = QStringLiteral("cleanup");
    entry.roots = QStringLiteral("%1 ok / %2 fail").arg(success).arg(fail);
    entry.filesFound = m_pendingCleanup.size();
    entry.trashedCount = success;
    entry.trashedBytes = bytesFreed;
    entry.errorCount = fail;
    addHistoryEntry(entry);

    const QStringList failures = m_cleanupFailures;
    m_cleanupFailures.clear();
    m_pendingCleanup.clear();
    updateTopState(QStringLiteral("state_idle"));

    QMessageBox box(this);
    box.setWindowTitle(i18n("cleanup_title"));
    box.setIcon(fail > 0 ? QMessageBox::Warning : QMessageBox::Information);
    QString text = i18n("cleanup_ok_line").arg(success).arg(I18n::formatBytes(bytesFreed));
    if (fail > 0) text += QStringLiteral("\n") + i18n("cleanup_fail_line").arg(fail);
    box.setText(text);
    if (!failures.isEmpty()) box.setDetailedText(failures.join(QStringLiteral("\n")));
    box.exec();
}

void MainWindow::refreshHistory() {
    m_historyPage->setHistory(m_history->entries());
    m_dashboard->setRecentHistory(m_history->entries());
}

void MainWindow::addHistoryEntry(const HistoryEntry& entry) {
    m_history->add(entry);
    refreshHistory();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_scanBusy || m_scanner->isBusy()) {
        m_scanner->cancel();
    }
    if (m_cleanupBusy || m_cleaner->isBusy()) {
        m_cleaner->cancel();
        event->ignore();
        return;
    }
    m_scanThread->quit();
    m_scanThread->wait(3000);
    m_cleanupThread->quit();
    m_cleanupThread->wait(3000);
    event->accept();
}
