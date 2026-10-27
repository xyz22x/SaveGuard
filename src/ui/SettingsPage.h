#pragma once
#include <QWidget>

class QPlainTextEdit;
class QSpinBox;
class QComboBox;
class QPushButton;
class QLabel;

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);
    void retranslate();

signals:
    void settingsSaved();

private:
    void buildUi();
    void loadFromSettings();
    void save();

    QPlainTextEdit* m_dirs = nullptr;
    QPlainTextEdit* m_exts = nullptr;
    QSpinBox* m_minSize = nullptr;
    QSpinBox* m_concurrency = nullptr;
    QComboBox* m_language = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QLabel* m_feedback = nullptr;
    QLabel* m_protectedNote = nullptr;
    QLabel* m_trashNote = nullptr;
};
