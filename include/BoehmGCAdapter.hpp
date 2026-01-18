#pragma once

#include "config.hpp"
#include <string>

namespace gc {

class BoehmGCAdapter : public MemoryManager {
private:
    MemoryStats stats;
public:
    BoehmGCAdapter();
    
    void* allocate(size_t size) override;
    void deallocate(void* ptr) override;
    void collect() override;
    
    MemoryStats getStats() const override;
    std::string name() const override;

    // Для Boehm эти методы пустые, так как он сам следит за ссылками
    // void addRef(void* ptr) override {}
    // void release(void* ptr) override {}
};

}