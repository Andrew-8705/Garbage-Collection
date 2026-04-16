#include "../../include/impl/ReferenceCountingGC.hpp"
#include "../../include/utils/logger.hpp"
#include <cstdlib>
#include <iostream>

#ifdef TRACY_ENABLE
    #include <tracy/Tracy.hpp>
#else
    #define TracyAllocN(ptr, size, name)
    #define TracyFreeN(ptr, name)
    #define TracyPlot(name, val)
    #define ZoneScoped
    #define ZoneValue(x)
#endif

#ifdef GC_THREAD_SAFE
    #define GC_LOCK_GUARD(m) std::lock_guard<std::mutex> lock(m)
#else
    #define GC_LOCK_GUARD(m)
#endif

namespace gc {

void* ReferenceCountingGC::allocate(size_t size) {
    ZoneScoped;
    ZoneValue(size);

    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }

    {
        GC_LOCK_GUARD(mtx);
        objects_map[ptr] = {0, size};
        stats.allocatedBytes += size;
        stats.liveBytes += size;
        stats.totalAllocations++;
    }

    TracyAllocN(ptr, size, "ReferenceCountingGC");
    TracyPlot("Live Memory (RC)", (int64_t)stats.liveBytes);

    Logger::getInstance().logAlloc(size, ptr);
    return ptr;
}

void ReferenceCountingGC::deallocate(void* ptr) {
    ZoneScoped;
    if (ptr == nullptr) return;

    size_t size_freed = 0;

    {
         GC_LOCK_GUARD(mtx);
        auto it = objects_map.find(ptr);
        if (it != objects_map.end()) {
            size_freed = it->second.size;
            
            if (stats.liveBytes >= size_freed) stats.liveBytes -= size_freed;
            stats.freedBytes += size_freed;
            
            objects_map.erase(it);
        } else {
            // Попытка удалить чужой указатель
            return; 
        }
    }

    TracyFreeN(ptr, "ReferenceCountingGC");
    TracyPlot("Live Memory (RC)", (int64_t)stats.liveBytes);

    std::free(ptr);
    Logger::getInstance().logFree(ptr);
}

void ReferenceCountingGC::collect() {
    // Logger::getInstance().log("GC: Collection requested (No-op for RC)");
}

MemoryStats ReferenceCountingGC::getStats() const {
    GC_LOCK_GUARD(mtx);
    return stats;
}

std::string ReferenceCountingGC::name() const {
    return "ReferenceCountingGC" 
#ifdef GC_THREAD_SAFE
     " (Multi-threaded)";
#else
     " (Single-threaded)";
#endif
}

void ReferenceCountingGC::addRef(void* ptr) {
    if (!ptr) return;
    GC_LOCK_GUARD(mtx);
    auto it = objects_map.find(ptr);
    if (it != objects_map.end()) {
        it->second.ref_count++;
    }
}

void ReferenceCountingGC::release(void* ptr) {
    if (!ptr) return;
    
    bool should_delete = false;
    
    {
        GC_LOCK_GUARD(mtx);
        auto it = objects_map.find(ptr);
        if (it != objects_map.end()) {
            if (it->second.ref_count > 0) {
                it->second.ref_count--;
            }
            
            if (it->second.ref_count == 0) {
                should_delete = true;   
            }
        }
    }

    if (should_delete) {
        deallocate(ptr);
    }
}

} // namespace gc