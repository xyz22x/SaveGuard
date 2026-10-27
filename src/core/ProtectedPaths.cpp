#include "ProtectedPaths.h"

#include <QDir>
#include <QVector>

static QVector<QString> protectedRoots() {
    QVector<QString> roots;
#ifdef Q_OS_WIN
    const QString windir = QDir::fromNativeSeparators(qEnvironmentVariable("SystemRoot", "C:/Windows"));
    roots << windir
          << "C:/Program Files"
          << "C:/Program Files (x86)"
          << "C:/ProgramData"
          << "C:/Users/Default";
#else
    roots << "/bin" << "/boot" << "/dev" << "/etc" << "/lib" << "/lib32" << "/lib64"
          << "/proc" << "/root" << "/run" << "/sbin" << "/srv" << "/sys" << "/usr" << "/var";
#endif
    return roots;
}

static QString normalize(QString p) {
    p = QDir::fromNativeSeparators(p);
    while (p.size() > 1 && p.endsWith('/')) p.chop(1);
    return p;
}

bool ProtectedPaths::isProtected(const QString& absolutePath) {
    const QString p = normalize(absolutePath);
    for (const QString& root : protectedRoots()) {
        const QString r = normalize(root);
        if (p.compare(r, Qt::CaseInsensitive) == 0) return true;
        if (p.startsWith(r + "/", Qt::CaseInsensitive)) return true;
    }
#ifdef Q_OS_WIN
    if (p.size() <= 3 && p.endsWith(':')) return true;
#else
    if (p == "/") return true;
#endif
    return false;
}

bool ProtectedPaths::isCriticalDir(const QString& dirName, const QString& absolutePath) {
    static const QVector<QString> names = {
        "$Recycle.Bin", "System Volume Information", "Windows", "Program Files",
        "Program Files (x86)", "ProgramData", "proc", "sys", "dev", "run", "boot"
    };
    if (names.contains(dirName, Qt::CaseInsensitive)) return true;
    if (isProtected(absolutePath)) return true;
    return false;
}
