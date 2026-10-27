#pragma once

#include "FileRecord.h"
#include "ScanTypes.h"
#include <functional>
#include <vector>

class DuplicateDetector {
public:
    struct GroupingOutput {
        QVector<DuplicateGroup> groups;
        qint64 hashErrors = 0;
        qint64 changedFiles = 0;
        bool canceled = false;
    };

    static GroupingOutput group(std::vector<FileRecord> records, int maxConcurrent,
                                const std::function<bool()>& shouldCancel,
                                const std::function<bool()>& shouldContinue,
                                const std::function<void(int, int)>& hashProgress);
};
