#pragma once

#include <QString>
#include <QByteArray>

#ifdef Q_OS_WIN
#include <filesystem>
inline std::filesystem::path toFsPath(const QString& s) { return std::filesystem::path(s.toStdWString()); }
inline QString fromFsPath(const std::filesystem::path& p) { return QString::fromStdWString(p.wstring()); }
#else
#include <filesystem>
inline std::filesystem::path toFsPath(const QString& s) { return std::filesystem::path(s.toStdString()); }
inline QString fromFsPath(const std::filesystem::path& p) { return QString::fromStdString(p.string()); }
#endif

class Hasher {
public:
    enum class Status { Ok, Failed, Canceled };

    static constexpr qint64 kMiniLimit = 64 * 1024;
    static constexpr qint64 kChunk = 1024 * 1024;

    static Status hashFile(const QString& path, qint64 limitBytes, QByteArray* out,
                           const std::function<bool()>& shouldAbort = {});

    static Status mini(const QString& path, QByteArray* out,
                       const std::function<bool()>& shouldAbort = {}) {
        return hashFile(path, kMiniLimit, out, shouldAbort);
    }

    static Status full(const QString& path, QByteArray* out,
                       const std::function<bool()>& shouldAbort = {}) {
        return hashFile(path, -1, out, shouldAbort);
    }
};

#include <functional>
