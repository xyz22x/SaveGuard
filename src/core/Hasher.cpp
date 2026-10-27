#include "Hasher.h"

#include <QCryptographicHash>
#include <QFile>
#include <QSaveFile>

Hasher::Status Hasher::hashFile(const QString& path, qint64 limitBytes, QByteArray* out,
                                const std::function<bool()>& shouldAbort) {
    if (!out) return Status::Failed;
    out->clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return Status::Failed;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    qint64 remaining = limitBytes < 0 ? file.size() : qMin(limitBytes, file.size());
    while (remaining > 0) {
        if (shouldAbort && shouldAbort()) return Status::Canceled;
        const qint64 toRead = qMin(remaining, kChunk);
        const QByteArray data = file.read(toRead);
        if (data.isEmpty()) return Status::Failed;
        hash.addData(data);
        remaining -= data.size();
    }
    *out = hash.result();
    return Status::Ok;
}
