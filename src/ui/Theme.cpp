#include "Theme.h"

#include <QStyle>
#include <QWidget>

void Theme::apply(QWidget* widget) {
    widget->setStyleSheet(qss());
}

void Theme::setClass(QWidget* widget, const QString& cls) {
    widget->setProperty("cls", cls);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

void Theme::unsetClass(QWidget* widget, const QString& cls) {
    Q_UNUSED(cls);
    widget->setProperty("cls", QString());
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

QString Theme::qss() {
    return QStringLiteral(R"(QMainWindow, QDialog {
    background-color: #0d0d10;
}

QWidget {
    background-color: #0d0d10;
    color: #e6e6ea;
    font-size: 10pt;
}

QLabel {
    background: transparent;
    color: #e6e6ea;
}

QLabel[cls="brand"] {
    font-size: 15pt;
    font-weight: 700;
    color: #fafafa;
}

QLabel[cls="brandSub"] {
    color: #8f8f9c;
    font-size: 9pt;
}

QLabel[cls="h1"] {
    font-size: 14pt;
    font-weight: 700;
    color: #fafafa;
}

QLabel[cls="h2"] {
    font-size: 11pt;
    font-weight: 600;
    color: #d9d9e0;
}

QLabel[cls="muted"] {
    color: #9a9aa5;
}

QLabel[cls="statValue"] {
    font-size: 16pt;
    font-weight: 700;
    color: #a78bfa;
}

QLabel[cls="statLabel"] {
    color: #9a9aa5;
    font-size: 9pt;
}

QLabel[cls="ok"] { color: #34d399; }
QLabel[cls="warn"] { color: #fbbf24; }
QLabel[cls="err"] { color: #f87171; }
QLabel[cls="accent"] { color: #a78bfa; }

QFrame[cls="line"] {
    border: none;
    border-top: 1px solid #232330;
    background: transparent;
}

QFrame[cls="card"] {
    background-color: #14141a;
    border: 1px solid #232330;
    border-radius: 8px;
}

QListWidget[cls="sidebar"] {
    background-color: transparent;
    border: none;
    color: #b7b7c2;
    font-size: 10.5pt;
    outline: 0;
}

QListWidget[cls="sidebar"]::item {
    padding: 10px 12px;
    border-radius: 6px;
    margin: 2px 6px;
}

QListWidget[cls="sidebar"]::item:hover {
    background-color: #17171e;
}

QListWidget[cls="sidebar"]::item:selected {
    background-color: #221d33;
    color: #c4b5fd;
}

QListWidget[cls="sidebar"]::item:selected:active {
    background-color: #221d33;
}

QPushButton {
    background-color: #1a1a22;
    border: 1px solid #26262f;
    border-radius: 6px;
    padding: 7px 14px;
    color: #d9d9e0;
}

QPushButton:hover {
    background-color: #22222b;
    border-color: #33333f;
}

QPushButton:pressed {
    background-color: #26262f;
}

QPushButton:focus {
    border-color: #8b5cf6;
}

QPushButton:disabled {
    color: #5a5a66;
    border-color: #1e1e26;
    background-color: #16161c;
}

QPushButton[cls="primary"] {
    background-color: #7c3aed;
    border-color: #7c3aed;
    color: #ffffff;
    font-weight: 600;
}

QPushButton[cls="primary"]:hover {
    background-color: #8b5cf6;
    border-color: #8b5cf6;
}

QPushButton[cls="primary"]:disabled {
    background-color: #3b3352;
    border-color: #3b3352;
    color: #8d87a3;
}

QPushButton[cls="danger"] {
    background-color: #2a1416;
    border-color: #7f1d1d;
    color: #f87171;
}

QPushButton[cls="danger"]:hover {
    background-color: #3d1a1d;
    border-color: #b91c1c;
}

QPushButton[cls="danger"]:disabled {
    color: #6b4a4d;
    border-color: #2a1a1c;
    background-color: #1a1214;
}

QPushButton[cls="ghost"] {
    background: transparent;
    border-color: transparent;
    color: #9a9aa5;
}

QPushButton[cls="ghost"]:hover {
    color: #e6e6ea;
    background-color: #1a1a22;
}

QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QComboBox {
    background-color: #14141a;
    border: 1px solid #26262f;
    border-radius: 6px;
    padding: 6px 8px;
    color: #e6e6ea;
    selection-background-color: #7c3aed;
    selection-color: #ffffff;
}

QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {
    border-color: #8b5cf6;
}

QLineEdit:disabled, QPlainTextEdit:disabled, QSpinBox:disabled, QComboBox:disabled {
    color: #5a5a66;
    background-color: #121216;
}

QComboBox::drop-down {
    border: none;
    width: 22px;
}

QComboBox QAbstractItemView {
    background-color: #14141a;
    border: 1px solid #26262f;
    color: #e6e6ea;
    selection-background-color: #221d33;
    selection-color: #c4b5fd;
}

QProgressBar {
    background-color: #14141a;
    border: 1px solid #26262f;
    border-radius: 6px;
    height: 14px;
    text-align: center;
    color: #b7b7c2;
    font-size: 8pt;
}

QProgressBar::chunk {
    background-color: #7c3aed;
    border-radius: 5px;
}

QProgressBar[cls="indeterminate"]::chunk {
    margin: 1px;
}

QTreeWidget, QTableWidget, QTreeView, QTableView, QListView {
    background-color: #101014;
    alternate-background-color: #12121a;
    border: 1px solid #232330;
    border-radius: 6px;
    color: #d9d9e0;
    outline: 0;
}

QTreeWidget::item, QTreeView::item, QTableView::item {
    padding: 4px;
}

QTreeWidget::item:hover, QTreeView::item:hover, QTableView::item:hover {
    background-color: #17171f;
}

QTreeWidget::item:selected, QTreeView::item:selected, QTableView::item:selected, QListView::item:selected {
    background-color: #221d33;
    color: #e9e4fb;
}

QHeaderView::section {
    background-color: #14141a;
    color: #9a9aa5;
    border: none;
    border-bottom: 1px solid #232330;
    padding: 6px 8px;
    font-weight: 600;
}

QTableCornerButton::section {
    background-color: #14141a;
    border: none;
}

QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}

QScrollBar::handle:vertical {
    background: #2b2b36;
    border-radius: 4px;
    min-height: 24px;
}

QScrollBar::handle:vertical:hover {
    background: #3a3a48;
}

QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 2px;
}

QScrollBar::handle:horizontal {
    background: #2b2b36;
    border-radius: 4px;
    min-width: 24px;
}

QScrollBar::handle:horizontal:hover {
    background: #3a3a48;
}

QScrollBar::add-line, QScrollBar::sub-line {
    height: 0;
    width: 0;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: transparent;
}

QToolTip {
    background-color: #1a1a22;
    color: #e6e6ea;
    border: 1px solid #33333f;
    padding: 5px;
    border-radius: 4px;
}

QCheckBox {
    color: #d9d9e0;
    spacing: 7px;
}

QCheckBox::indicator {
    width: 15px;
    height: 15px;
    border: 1px solid #3a3a48;
    border-radius: 4px;
    background-color: #14141a;
}

QCheckBox::indicator:checked {
    background-color: #7c3aed;
    border-color: #7c3aed;
    image: url(:/icons/check.svg);
}

QMenu {
    background-color: #16161c;
    border: 1px solid #26262f;
    padding: 4px;
}

QMenu::item {
    padding: 6px 22px;
    border-radius: 4px;
    color: #d9d9e0;
}

QMenu::item:selected {
    background-color: #221d33;
    color: #c4b5fd;
}

QMenu::separator {
    height: 1px;
    background: #26262f;
    margin: 4px 6px;
}

QStatusBar {
    background: #101014;
    color: #9a9aa5;
    border-top: 1px solid #232330;
}

QGroupBox {
    border: 1px solid #232330;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 6px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: #9a9aa5;
}
)");
}
