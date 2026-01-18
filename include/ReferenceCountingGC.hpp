#pragma once

#include "config.hpp"
#include <unordered_map>
#include <mutex>
#include <string>

namespace gc {

class ReferenceCountingGC : public MemoryManager {
private:

    struct ObjectInfo {
        size_t ref_count = 0;
        size_t size = 0;
    };

    std::unordered_map<void*, ObjectInfo> objects_map;
    mutable std::mutex mtx;
    MemoryStats stats;

public:

    void* allocate(size_t size) override;
    void deallocate(void* ptr) override;
    void collect() override;
    
    MemoryStats getStats() const override;
    std::string name() const override;

    void addRef(void* ptr) override;
    void release(void* ptr) override;
};

} // namespace gc