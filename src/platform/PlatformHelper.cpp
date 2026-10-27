#include "PlatformHelper.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>

static bool shellSelect(const QString& nativePath) {
    const std::wstring w = nativePath.toStdWString();
    PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(w.c_str());
    if (!pidl) return false;
    const HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    ILFree(pidl);
    return SUCCEEDED(hr);
}
#endif

bool PlatformHelper::openFile(const QString& path) {
    return QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::toNativeSeparators(path)));
}

bool PlatformHelper::revealInShell(const QString& path) {
#ifdef Q_OS_WIN
    return shellSelect(QDir::toNativeSeparators(path));
#else
    const QFileInfo info(path);
    return QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
#endif
}
