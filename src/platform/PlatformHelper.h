#pragma once

class QString;

class PlatformHelper {
public:
    static bool openFile(const QString& path);
    static bool revealInShell(const QString& path);
};
