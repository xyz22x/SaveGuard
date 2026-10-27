#pragma once

#include <QString>

class ProtectedPaths {
public:
    static bool isProtected(const QString& absolutePath);
    static bool isCriticalDir(const QString& dirName, const QString& absolutePath);
};
