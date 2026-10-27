#pragma once

#include <QObject>
#include <QStringList>

class Settings : public QObject {
    Q_OBJECT
public:
    static Settings& instance();

    void load();
    void save();

    QStringList ignoredDirs() const { return m_ignoredDirs; }
    QStringList ignoredExtensions() const { return m_ignoredExtensions; }
    qint64 minFileSize() const { return m_minFileSize; }
    int maxConcurrent() const { return m_maxConcurrent; }
    QString language() const { return m_language; }

    void setIgnoredDirs(const QStringList& v);
    void setIgnoredExtensions(const QStringList& v);
    void setMinFileSize(qint64 bytes);
    void setMaxConcurrent(int v);
    void setLanguage(const QString& lang);

private:
    Settings();

    QStringList m_ignoredDirs;
    QStringList m_ignoredExtensions;
    qint64 m_minFileSize = 0;
    int m_maxConcurrent = 4;
    QString m_language = QStringLiteral("pt_BR");
};
