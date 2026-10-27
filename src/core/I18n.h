#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <utility>

class I18n : public QObject {
    Q_OBJECT
public:
    static I18n& instance();

    QString t(const QString& key) const;
    bool isEnglish() const { return m_language == QStringLiteral("en"); }
    void setLanguage(const QString& lang);
    QString language() const { return m_language; }

    static QString formatBytes(qint64 bytes);

signals:
    void languageChanged();

private:
    I18n();

    QString m_language;
    QHash<QString, QPair<QString, QString>> m_table;
};

#define i18n(key) I18n::instance().t(QStringLiteral(key))
