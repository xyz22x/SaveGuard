#pragma once

#include <QWidget>

class Theme {
public:
    static void apply(QWidget* widget);
    static void setClass(QWidget* widget, const QString& cls);
    static void unsetClass(QWidget* widget, const QString& cls);
    static QString qss();
};
