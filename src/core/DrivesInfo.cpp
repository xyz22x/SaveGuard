#include "DrivesInfo.h"

#include <QDir>
#include <QStorageInfo>

QVector<DriveInfo> DrivesInfo::mounted() {
    QVector<DriveInfo> out;
    const auto volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& v : volumes) {
        if (!v.isValid() || !v.isReady() || v.bytesTotal() <= 0) continue;
        const QString fsType = QString::fromLatin1(v.fileSystemType()).toLower();
        static const QStringList pseudo = {QStringLiteral("tmpfs"), QStringLiteral("devtmpfs"),
                                           QStringLiteral("squashfs"), QStringLiteral("overlay"),
                                           QStringLiteral("proc"), QStringLiteral("sysfs"),
                                           QStringLiteral("udev"), QStringLiteral("ramfs")};
        if (pseudo.contains(fsType)) continue;
        DriveInfo info;
        info.path = QDir::fromNativeSeparators(v.rootPath());
        info.name = v.name();
        info.fileSystem = fsType;
        info.totalBytes = v.bytesTotal();
        info.freeBytes = v.bytesFree();
        out.append(info);
    }
    return out;
}
