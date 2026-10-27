#include "Settings.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>

Settings::Settings() {
    load();
}

Settings& Settings::instance() {
    static Settings s;
    return s;
}

void Settings::load() {
    QSettings st(QSettings::IniFormat, QSettings::UserScope,
                 QStringLiteral("SaveGuard"), QStringLiteral("SaveGuard"));
    m_ignoredDirs = st.value("ignoredDirs").toStringList();
    m_ignoredExtensions = st.value("ignoredExtensions").toStringList();
    m_minFileSize = static_cast<qint64>(st.value("minFileSize", 0).toLongLong());
    m_maxConcurrent = st.value("maxConcurrent", 4).toInt();
    m_language = st.value("language", "pt_BR").toString();
    if (m_maxConcurrent < 1) m_maxConcurrent = 1;
    if (m_maxConcurrent > 16) m_maxConcurrent = 16;
}

void Settings::save() {
    QSettings st(QSettings::IniFormat, QSettings::UserScope,
                 QStringLiteral("SaveGuard"), QStringLiteral("SaveGuard"));
    st.setValue("ignoredDirs", m_ignoredDirs);
    st.setValue("ignoredExtensions", m_ignoredExtensions);
    st.setValue("minFileSize", m_minFileSize);
    st.setValue("maxConcurrent", m_maxConcurrent);
    st.setValue("language", m_language);
    st.sync();
}

void Settings::setIgnoredDirs(const QStringList& v) { m_ignoredDirs = v; }
void Settings::setIgnoredExtensions(const QStringList& v) { m_ignoredExtensions = v; }
void Settings::setMinFileSize(qint64 bytes) { m_minFileSize = qMax<qint64>(0, bytes); }
void Settings::setMaxConcurrent(int v) { m_maxConcurrent = qBound(1, v, 16); }
void Settings::setLanguage(const QString& lang) { m_language = lang; }
