#include "ReviewDialog.h"

#include "I18n.h"
#include "Theme.h"

#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ReviewDialog::ReviewDialog(const QVector<CleanupItem>& items, QWidget* parent)
    : QDialog(parent) {
    setModal(true);
    setMinimumSize(640, 460);
    Theme::apply(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 16);
    layout->setSpacing(10);

    auto* title = new QLabel(i18n("review_title"), this);
    Theme::setClass(title, "h1");
    auto* subtitle = new QLabel(i18n("review_subtitle"), this);
    Theme::setClass(subtitle, "muted");
    subtitle->setWordWrap(true);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({i18n("review_col_path"), i18n("review_col_size")});
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    qint64 total = 0;
    m_table->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        auto* pathItem = new QTableWidgetItem(QDir::toNativeSeparators(items[i].path));
        pathItem->setToolTip(items[i].path);
        m_table->setItem(i, 0, pathItem);
        auto* sizeItem = new QTableWidgetItem(I18n::formatBytes(items[i].size));
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(i, 1, sizeItem);
        total += items[i].size;
    }
    m_table->setSortingEnabled(false);

    m_total = new QLabel(i18n("review_total").arg(items.size()).arg(I18n::formatBytes(total)), this);
    Theme::setClass(m_total, "h2");

    auto* note = new QLabel(i18n("review_validate_note"), this);
    Theme::setClass(note, "muted");
    note->setWordWrap(true);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch(1);
    auto* cancelBtn = new QPushButton(i18n("btn_close"), this);
    auto* okBtn = new QPushButton(i18n("btn_send_trash"), this);
    Theme::setClass(okBtn, "danger");
    okBtn->setDefault(true);
    buttons->addWidget(cancelBtn);
    buttons->addWidget(okBtn);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_total);
    layout->addWidget(note);
    layout->addLayout(buttons);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}
