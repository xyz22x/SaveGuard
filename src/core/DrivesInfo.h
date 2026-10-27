#pragma once

#include <QString>
#include <QVector>

struct DriveInfo {
    QString path;
    QString name;
    QString fileSystem;
    qint64 totalBytes = 0;
    qint64 freeBytes = 0;
};

class DrivesInfo {
public:
    static QVector<DriveInfo> mounted();
};
