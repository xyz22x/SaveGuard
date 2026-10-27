#include "HistoryStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

HistoryStore::HistoryStore(QString filePath) : m_filePath(std::move(filePath)) {}

QString HistoryStore::defaultFilePath() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + QStringLiteral("/history.json");
}

QVector<HistoryEntry> HistoryStore::entries() const {
    QVector<HistoryEntry> out;
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) return out;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonArray arr = doc.array();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        HistoryEntry e;
        e.timestampMs = static_cast<qint64>(o.value("ts").toDouble());
        e.durationMs = static_cast<qint64>(o.value("dur").toDouble());
        e.type = o.value("type").toString();
        e.roots = o.value("roots").toString();
        e.filesFound = static_cast<qint64>(o.value("files").toDouble());
        e.recoverableBytes = static_cast<qint64>(o.value("rec").toDouble());
        e.trashedCount = static_cast<qint64>(o.value("trashed").toDouble());
        e.trashedBytes = static_cast<qint64>(o.value("trashedBytes").toDouble());
        e.errorCount = static_cast<qint64>(o.value("errors").toDouble());
        out.append(e);
    }
    return out;
}

void HistoryStore::add(const HistoryEntry& entry) {
    QVector<HistoryEntry> all = entries();
    all.append(entry);
    if (all.size() > 500) all = all.mid(all.size() - 500);

    QJsonArray arr;
    for (const HistoryEntry& e : all) {
        QJsonObject o;
        o.insert("ts", e.timestampMs);
        o.insert("dur", e.durationMs);
        o.insert("type", e.type);
        o.insert("roots", e.roots);
        o.insert("files", e.filesFound);
        o.insert("rec", e.recoverableBytes);
        o.insert("trashed", e.trashedCount);
        o.insert("trashedBytes", e.trashedBytes);
        o.insert("errors", e.errorCount);
        arr.append(o);
    }

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void HistoryStore::clear() {
    QFile::remove(m_filePath);
}
