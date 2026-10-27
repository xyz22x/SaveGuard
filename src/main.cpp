#include "CleanupManager.h"
#include "DashboardPage.h"
#include "DuplicatesPage.h"
#include "BigFilesPage.h"
#include "HistoryPage.h"
#include "HistoryStore.h"
#include "I18n.h"
#include "MainWindow.h"
#include "ScanTypes.h"
#include "Settings.h"
#include "Theme.h"

#include <QApplication>
#include <QFile>
#include <QIcon>

int main(int argc, char* argv[]) {
    qRegisterMetaType<ScanConfig>("ScanConfig");
    qRegisterMetaType<ScanStats>("ScanStats");
    qRegisterMetaType<FileRecord>("FileRecord");
    qRegisterMetaType<QVector<FileRecord>>("QVector<FileRecord>");
    qRegisterMetaType<DuplicateGroup>("DuplicateGroup");
    qRegisterMetaType<QVector<DuplicateGroup>>("QVector<DuplicateGroup>");
    qRegisterMetaType<CleanupItem>("CleanupItem");
    qRegisterMetaType<QVector<CleanupItem>>("QVector<CleanupItem>");
    qRegisterMetaType<CleanupResult>("CleanupResult");
    qRegisterMetaType<ScanMode>("ScanMode");

    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("SaveGuard"));
    QApplication::setApplicationName(QStringLiteral("SaveGuard"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    app.setWindowIcon(QIcon(":/icons/shield.svg"));

    app.setStyleSheet(Theme::qss());

    MainWindow window;
    QObject::connect(&I18n::instance(), &I18n::languageChanged, &window, &MainWindow::retranslateAll);
    window.show();

    return app.exec();
}
