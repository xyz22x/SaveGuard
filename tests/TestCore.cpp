#include "CleanupManager.h"
#include "DuplicateDetector.h"
#include "Hasher.h"
#include "HistoryStore.h"
#include "Scanner.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QProcess>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace {

void writeFile(const QString& path, const QByteArray& content) {
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(f.write(content) == content.size());
}

QByteArray repeatedContent(int chunks, char seed) {
    QByteArray data;
    data.reserve(chunks * 2048);
    for (int i = 0; i < chunks * 2048; ++i) {
        data.append(char(seed + (i % 251)));
    }
    return data;
}

QString norm(const QString& p) { return QDir::fromNativeSeparators(p); }

QVector<FileRecord> runScan(const QString& root, ScanStats* statsOut = nullptr) {
    ScanConfig config;
    config.roots = {root};
    config.maxConcurrent = 2;
    QVector<FileRecord> records;
    ScanStats stats;
    Scanner scanner;
    QObject::connect(&scanner, &Scanner::batch,
                     [&](const QVector<FileRecord>& batch) {
                         for (const FileRecord& rec : batch) records.append(rec);
                     });
    QObject::connect(&scanner, &Scanner::finishedStats,
                     [&](const ScanStats& s) { stats = s; });
    scanner.run(config);
    if (statsOut) *statsOut = stats;
    return records;
}

DuplicateDetector::GroupingOutput groupAll(const QVector<FileRecord>& records) {
    return DuplicateDetector::group(std::vector<FileRecord>(records.begin(), records.end()),
                                    2, []() { return false; }, []() { return true; }, {});
}

}

class TestCore : public QObject {
    Q_OBJECT

private slots:
    void duplicatesGrouping();
    void emptyFilesGroup();
    void sameSizeDifferentContent();
    void nestedFolders();
    void unicodeAndSpaces();
    void recoverableMath();
    void symlinksSkipped();
    void cancelScan();
    void inaccessibleFile();
    void changedFileRejectedByRevalidation();
    void trashFailureOnMissingFile();
    void protectedPathRefused();
    void cleanupSuccessMovesToTrash();
    void historyRoundTrip();
    void hasherCancel();
};

void TestCore::duplicatesGrouping() {
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString base = norm(root.path());
    QVERIFY(QDir(base + "/a").mkpath("."));
    const QByteArray content = repeatedContent(8, 'x');
    writeFile(base + "/a/copy_one.dat", content);
    writeFile(base + "/a/copy_two.dat", content);

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 2);

    const auto out = groupAll(records);
    QCOMPARE(out.canceled, false);
    QCOMPARE(out.hashErrors, qint64(0));
    QCOMPARE(out.groups.size(), 1);
    QCOMPARE(out.groups.first().files.size(), 2);
    QCOMPARE(out.groups.first().size, qint64(content.size()));
}

void TestCore::emptyFilesGroup() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    writeFile(base + "/empty_a.txt", QByteArray());
    writeFile(base + "/empty_b.txt", QByteArray());

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 2);

    const auto out = groupAll(records);
    QCOMPARE(out.groups.size(), 1);
    QCOMPARE(out.groups.first().recoverable(), qint64(0));
}

void TestCore::sameSizeDifferentContent() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    writeFile(base + "/x1.bin", repeatedContent(4, 'a'));
    writeFile(base + "/x2.bin", repeatedContent(4, 'b'));

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 2);
    QCOMPARE(records[0].size, records[1].size);

    const auto out = groupAll(records);
    QCOMPARE(out.groups.size(), 0);
}

void TestCore::nestedFolders() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    QVERIFY(QDir(base + "/l1/l2/l3/l4").mkpath("."));
    const QByteArray content = repeatedContent(3, 'n');
    writeFile(base + "/l1/top.dat", content);
    writeFile(base + "/l1/l2/l3/l4/deep.dat", content);

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 2);

    const auto out = groupAll(records);
    QCOMPARE(out.groups.size(), 1);
    QCOMPARE(out.groups.first().files.size(), 2);
}

void TestCore::unicodeAndSpaces() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QString folder = base + "/pasta çãõ com espaços 文件夹";
    QVERIFY(QDir(folder).mkpath("."));
    const QByteArray content = repeatedContent(2, 'u');
    writeFile(folder + "/arquivo especial çåœ 日本語.txt", content);
    writeFile(base + "/outra copia çåœ 日本語.txt", content);

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 2);

    const auto out = groupAll(records);
    QCOMPARE(out.groups.size(), 1);
    for (const FileRecord& rec : out.groups.first().files) {
        QVERIFY(rec.path.contains(QStringLiteral("çãõ")) || rec.path.contains(QStringLiteral("çåœ")));
    }
}

void TestCore::recoverableMath() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QByteArray content(1000, 'r');
    writeFile(base + "/r1.bin", content);
    writeFile(base + "/r2.bin", content);
    writeFile(base + "/r3.bin", content);

    const QVector<FileRecord> records = runScan(base);
    QCOMPARE(records.size(), 3);

    const auto out = groupAll(records);
    QCOMPARE(out.groups.size(), 1);
    QCOMPARE(out.groups.first().files.size(), 3);
    QCOMPARE(out.groups.first().recoverable(), qint64(2000));
}

void TestCore::symlinksSkipped() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    writeFile(base + "/original.txt", repeatedContent(2, 's'));

    const std::error_code ec = [&]() {
        std::error_code e;
        fs::create_symlink(toFsPath(base + "/original.txt"),
                           toFsPath(base + "/link.txt"), e);
        return e;
    }();
    if (ec) QSKIP("symlink creation unsupported in this environment");

    ScanStats stats;
    const QVector<FileRecord> records = runScan(base, &stats);
    QCOMPARE(stats.symlinksSkipped, qint64(1));
    QCOMPARE(records.size(), 1);
}

void TestCore::cancelScan() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    QVERIFY(QDir(base + "/bulk").mkpath("."));
    for (int i = 0; i < 2000; ++i) {
        writeFile(base + QString("/bulk/f%1.dat").arg(i), QByteArray(512, char(i % 97)));
    }

    ScanConfig config;
    config.roots = {base};

    Scanner scanner;
    int batches = 0;
    QString finalState;
    QObject::connect(&scanner, &Scanner::batch, [&](const QVector<FileRecord>&) {
        if (++batches == 2) scanner.cancel();
    });
    QObject::connect(&scanner, &Scanner::finishedStats, [&](const ScanStats& s) {
        finalState = s.state;
    });
    scanner.run(config);
    QCOMPARE(finalState, QStringLiteral("canceled"));
    QVERIFY(batches >= 2);
    QVERIFY(!scanner.isBusy());
}

void TestCore::inaccessibleFile() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QString sub = base + "/restricted";
    QVERIFY(QDir(sub).mkpath("."));
    writeFile(sub + "/inner.dat", QByteArray(2048, 'l'));

#ifdef Q_OS_WIN
    QProcess icacls;
    icacls.start(QStringLiteral("icacls"), QStringList{} << sub << QStringLiteral("/deny") << QStringLiteral("*S-1-1-0:(OI)(CI)R"));
    if (!icacls.waitForFinished(5000) || icacls.exitCode() != 0) QSKIP("icacls deny unavailable");
#else
    if (chmod(qPrintable(sub), 0) != 0) QSKIP("chmod unsupported");
#endif

    ScanStats stats;
    runScan(base, &stats);

#ifdef Q_OS_WIN
    QProcess restore;
    restore.start(QStringLiteral("icacls"), QStringList{} << sub << QStringLiteral("/reset") << QStringLiteral("*S-1-1-0"));
    restore.waitForFinished(5000);
#else
    chmod(qPrintable(sub), 0755);
#endif
    QVERIFY2(stats.errors >= 1, qPrintable(QString::number(stats.errors)));
}

void TestCore::changedFileRejectedByRevalidation() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QString path = base + "/changing.bin";
    const QByteArray first = repeatedContent(2, 'c');
    writeFile(path, first);

    CleanupItem item;
    item.path = path;
    item.size = first.size();
    item.mtimeSec = QFileInfo(path).lastModified().toSecsSinceEpoch();
    QByteArray mini;
    QCOMPARE(Hasher::mini(path, &mini), Hasher::Status::Ok);
    item.miniHash = mini;

    writeFile(path, repeatedContent(3, 'd'));

    CleanupManager manager;
    int fail = 0;
    QString reason;
    QObject::connect(&manager, &CleanupManager::itemResult, [&](const CleanupResult& r) {
        if (!r.success) {
            fail++;
            reason = r.error;
        }
    });
    manager.run(QVector<CleanupItem>{item}, true);
    QCOMPARE(fail, 1);
    QVERIFY2(reason == QStringLiteral("cleanup_err_changed_size")
                 || reason == QStringLiteral("cleanup_err_changed_mtime")
                 || reason == QStringLiteral("cleanup_err_changed_content"),
             qPrintable(reason));
    QVERIFY(QFileInfo::exists(path));
}

void TestCore::trashFailureOnMissingFile() {
    CleanupManager manager;
    int fail = 0;
    QString reason;
    QObject::connect(&manager, &CleanupManager::itemResult, [&](const CleanupResult& r) {
        if (!r.success) {
            fail++;
            reason = r.error;
        }
    });
    manager.run(QVector<CleanupItem>{CleanupItem{"Z:/nao/existe/absolutamente.bin", 10, 0, {}}}, true);
    QCOMPARE(fail, 1);
    QCOMPARE(reason, QStringLiteral("cleanup_err_missing"));
}

void TestCore::protectedPathRefused() {
#ifdef Q_OS_WIN
    const QString target = QStringLiteral("C:/Windows/win.ini");
    if (!QFileInfo::exists(target)) QSKIP("windows dir not present");
#else
    const QString target = QStringLiteral("/etc/hostname");
    if (!QFileInfo::exists(target)) QSKIP("etc file not present");
#endif
    const qint64 before = QFileInfo(target).size();

    CleanupManager manager;
    int fail = 0;
    QString reason;
    QObject::connect(&manager, &CleanupManager::itemResult, [&](const CleanupResult& r) {
        if (!r.success) {
            fail++;
            reason = r.error;
        }
    });
    manager.run(QVector<CleanupItem>{CleanupItem{target, before, 0, {}}}, true);
    QCOMPARE(fail, 1);
    QCOMPARE(reason, QStringLiteral("cleanup_err_protected"));
    QVERIFY(QFileInfo::exists(target));
    QCOMPARE(QFileInfo(target).size(), before);
}

void TestCore::cleanupSuccessMovesToTrash() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QString path = base + "/trashme_unique_sg.bin";
    writeFile(path, QByteArray(4096, 't'));

    CleanupItem item;
    item.path = path;
    item.size = 4096;
    item.mtimeSec = QFileInfo(path).lastModified().toSecsSinceEpoch();
    QByteArray mini;
    QCOMPARE(Hasher::mini(path, &mini), Hasher::Status::Ok);
    item.miniHash = mini;

    CleanupManager manager;
    int success = 0;
    int fail = 0;
    QString failReason;
    QObject::connect(&manager, &CleanupManager::itemResult, [&](const CleanupResult& r) {
        if (r.success) success++;
        else {
            fail++;
            failReason = r.error;
        }
    });
    manager.run(QVector<CleanupItem>{item}, true);

    if (success == 1) {
        QVERIFY(!QFileInfo::exists(path));
    } else {
        QVERIFY(QFileInfo::exists(path));
        QSKIP(qPrintable(QStringLiteral("trash unavailable in this environment: %1").arg(failReason)));
    }
}

void TestCore::historyRoundTrip() {
    QTemporaryDir tmp;
    const QString file = norm(tmp.path()) + "/hist/history.json";
    HistoryStore store(file);

    HistoryEntry e;
    e.timestampMs = 1730000000000;
    e.durationMs = 4200;
    e.type = "scan";
    e.roots = "C:/temp";
    e.filesFound = 12;
    e.recoverableBytes = 4096;
    e.trashedCount = 3;
    e.trashedBytes = 2048;
    e.errorCount = 1;
    store.add(e);

    const QVector<HistoryEntry> loaded = store.entries();
    QCOMPARE(loaded.size(), 1);
    QCOMPARE(loaded.first().filesFound, qint64(12));
    QCOMPARE(loaded.first().trashedCount, qint64(3));
    QCOMPARE(loaded.first().recoverableBytes, qint64(4096));

    store.clear();
    QVERIFY(store.entries().isEmpty());
}

void TestCore::hasherCancel() {
    QTemporaryDir root;
    const QString base = norm(root.path());
    const QString path = base + "/big.dat";
    writeFile(path, repeatedContent(64, 'z'));

    QByteArray out;
    const Hasher::Status canceled = Hasher::hashFile(path, -1, &out, []() { return true; });
    QCOMPARE(canceled, Hasher::Status::Canceled);
    QVERIFY(out.isEmpty());

    const Hasher::Status ok = Hasher::hashFile(path, -1, &out);
    QCOMPARE(ok, Hasher::Status::Ok);
    QCOMPARE(out.size(), 32);
}

QTEST_MAIN(TestCore)
#include "TestCore.moc"
