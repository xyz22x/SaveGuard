#include "SettingsPage.h"

#include "I18n.h"
#include "Settings.h"
#include "Theme.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {
    buildUi();
    loadFromSettings();
    retranslate();
}

void SettingsPage::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    auto* title = new QLabel(this);
    Theme::setClass(title, "h1");
    title->setObjectName("setTitle");

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(10);

    m_dirs = new QPlainTextEdit(this);
    m_dirs->setFixedHeight(96);
    auto* dirsHint = new QLabel(this);
    Theme::setClass(dirsHint, "muted");
    dirsHint->setObjectName("setDirsHint");
    dirsHint->setWordWrap(true);
    form->addRow(i18n("set_dirs"), m_dirs);
    form->addRow(QString(), dirsHint);

    m_exts = new QPlainTextEdit(this);
    m_exts->setFixedHeight(72);
    auto* extsHint = new QLabel(this);
    Theme::setClass(extsHint, "muted");
    extsHint->setObjectName("setExtsHint");
    extsHint->setWordWrap(true);
    form->addRow(i18n("set_exts"), m_exts);
    form->addRow(QString(), extsHint);

    m_minSize = new QSpinBox(this);
    m_minSize->setRange(0, 1024 * 100);
    m_minSize->setSuffix(" MB");
    form->addRow(i18n("set_min_size"), m_minSize);

    m_concurrency = new QSpinBox(this);
    m_concurrency->setRange(1, 16);
    form->addRow(i18n("set_concurrency"), m_concurrency);

    m_language = new QComboBox(this);
    m_language->addItem(i18n("set_lang_pt"), QStringLiteral("pt_BR"));
    m_language->addItem(i18n("set_lang_en"), QStringLiteral("en"));
    form->addRow(i18n("set_language"), m_language);

    m_protectedNote = new QLabel(this);
    m_protectedNote->setWordWrap(true);
    Theme::setClass(m_protectedNote, "muted");
    m_trashNote = new QLabel(this);
    m_trashNote->setWordWrap(true);
    Theme::setClass(m_trashNote, "muted");

    auto* controls = new QHBoxLayout;
    m_saveBtn = new QPushButton(this);
    Theme::setClass(m_saveBtn, "primary");
    m_feedback = new QLabel(this);
    Theme::setClass(m_feedback, "ok");
    controls->addWidget(m_saveBtn);
    controls->addWidget(m_feedback);
    controls->addStretch(1);

    layout->addWidget(title);
    layout->addLayout(form);
    layout->addWidget(m_protectedNote);
    layout->addWidget(m_trashNote);
    layout->addLayout(controls);
    layout->addStretch(1);

    connect(m_saveBtn, &QPushButton::clicked, this, [this]() { save(); });
}

void SettingsPage::loadFromSettings() {
    Settings& s = Settings::instance();
    m_dirs->setPlainText(s.ignoredDirs().join('\n'));
    m_exts->setPlainText(s.ignoredExtensions().join('\n'));
    m_minSize->setValue(int(s.minFileSize() / (1024 * 1024)));
    m_concurrency->setValue(s.maxConcurrent());
    const int idx = m_language->findData(s.language());
    m_language->setCurrentIndex(idx < 0 ? 0 : idx);
}

void SettingsPage::retranslate() {
    findChild<QLabel*>("setTitle")->setText(i18n("nav_settings"));
    findChild<QLabel*>("setDirsHint")->setText(i18n("set_dirs_hint"));
    findChild<QLabel*>("setExtsHint")->setText(i18n("set_exts_hint"));
    m_saveBtn->setText(i18n("btn_save"));
    m_protectedNote->setText(i18n("set_protected_note"));
    m_trashNote->setText(i18n("set_trash_note"));
    m_language->setItemText(0, i18n("set_lang_pt"));
    m_language->setItemText(1, i18n("set_lang_en"));
}

void SettingsPage::save() {
    Settings& s = Settings::instance();
    QStringList dirs;
    for (const QString& line : m_dirs->toPlainText().split('\n')) {
        const QString t = line.trimmed();
        if (!t.isEmpty()) dirs << t;
    }
    QStringList exts;
    for (const QString& line : m_exts->toPlainText().split('\n')) {
        QString t = line.trimmed().toLower();
        if (t.isEmpty()) continue;
        if (!t.startsWith('.')) t = "." + t;
        exts << t;
    }
    s.setIgnoredDirs(dirs);
    s.setIgnoredExtensions(exts);
    s.setMinFileSize(qint64(m_minSize->value()) * 1024 * 1024);
    s.setMaxConcurrent(m_concurrency->value());
    const QString lang = m_language->currentData().toString();
    s.setLanguage(lang);
    s.save();
    m_feedback->setText(i18n("set_saved"));
    if (I18n::instance().language() != lang) {
        I18n::instance().setLanguage(lang);
    }
    emit settingsSaved();
}
