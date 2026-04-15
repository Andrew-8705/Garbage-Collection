#include "../../include/impl/BoehmGCAdapter.hpp"
#include "../../include/utils/logger.hpp"
#include <gc.h> // API библиотеки Boehm

#ifdef TRACY_ENABLE
    #include <tracy/Tracy.hpp>
#else
    #define TracyAllocN(ptr, size, name)
    #define TracyFreeN(ptr, name)
    #define TracyPlot(name, val)
    #define ZoneScopedC(x)
#endif

namespace gc {

BoehmGCAdapter::BoehmGCAdapter() {
    GC_INIT();
}

void* BoehmGCAdapter::allocate(size_t size) {
    void* ptr = GC_MALLOC(size);
    
    if (ptr) {
        TracyAllocN(ptr, size, "BoehmGC");

        stats.totalAllocations++;
        stats.allocatedBytes += size; 

        TracyPlot("Total Allocated (Boehm)", (int64_t)stats.allocatedBytes);
    
        Logger::getInstance().logAlloc(size, ptr);        
    }
    return ptr;
}

void BoehmGCAdapter::deallocate(void* ptr) {
    if (ptr) {
        TracyFreeN(ptr, "BoehmGC");
        GC_FREE(ptr);
        Logger::getInstance().logFree(ptr);
    }
}

void BoehmGCAdapter::collect() {
    ZoneScopedC(0xFF0000); 

    Logger::getInstance().log("BoehmGC: Force Collect");

    GC_gcollect();
    stats.totalCollections++;

    size_t live = GC_get_heap_size() - GC_get_free_bytes();
    TracyPlot("Live Memory (Boehm)", (int64_t)live);
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