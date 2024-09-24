#ifndef SE_QUEUE_FAMILY_INDICES_H
#define SE_QUEUE_FAMILY_INDICES_H

#include <optional>

struct SeQueueFamilyIndices {
    std::optional<uint32_t> graphic_family;
    bool isComplete() {
        return graphic_family.has_value();
    };
};

#endif