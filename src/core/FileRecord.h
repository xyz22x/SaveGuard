#pragma once

#include <QString>
#include <QByteArray>
#include <QMetaType>

struct FileRecord {
    QString path;
    qint64 size = 0;
    qint64 mtimeSec = 0;
    QByteArray miniHash;
};

Q_DECLARE_METATYPE(FileRecord)
