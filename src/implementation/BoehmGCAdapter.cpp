#include "BoehmGCAdapter.hpp"
#include "logger.hpp"
#include <gc.h> // API библиотеки Boehm

namespace gc {

BoehmGCAdapter::BoehmGCAdapter() {
    GC_INIT();
}

void* BoehmGCAdapter::allocate(size_t size) {
    void* ptr = GC_MALLOC(size);
    
    if (ptr) {
        Logger::getInstance().logAlloc(size, ptr);

        stats.totalAllocations++;
        stats.allocatedBytes += size; 
    }
    return ptr;
}

void BoehmGCAdapter::deallocate(void* ptr) {
    if (ptr) {
        GC_FREE(ptr);
        Logger::getInstance().logFree(ptr);
    }
}

void BoehmGCAdapter::collect() {
    // Принудительный запуск сборки
    Logger::getInstance().log("BoehmGC: Force Collect");
    GC_gcollect();
    stats.totalCollections++;
}

MemoryStats BoehmGCAdapter::getStats() const {
    MemoryStats result = stats;

    size_t heapSize = GC_get_heap_size();
    size_t freeBytes = GC_get_free_bytes();
    
    result.liveBytes = heapSize - freeBytes;

    return result;
}

std::string BoehmGCAdapter::name() const {
    return "Boehm-Demers-Weiser GC";
}

} // namespace gc