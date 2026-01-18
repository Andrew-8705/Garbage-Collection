#include "ReferenceCountingGC.hpp"
#include "logger.hpp"
#include <cstdlib>
#include <iostream>

namespace gc {

void* ReferenceCountingGC::allocate(size_t size) {
    // 1. Системная аллокация
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }

    // 2. Регистрация в карте (под мьютексом)
    {
        std::lock_guard<std::mutex> lock(mtx);
        
        objects_map[ptr] = {0, size};

        stats.allocatedBytes += size;
        stats.liveBytes += size;
        stats.totalAllocations++;
    }

    // 3. Логирование
    Logger::getInstance().logAlloc(size, ptr);
    return ptr;
}

void ReferenceCountingGC::deallocate(void* ptr) {
    if (ptr == nullptr) return;

    size_t size_freed = 0;

    // 1. Удаление из карты
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = objects_map.find(ptr);
        if (it != objects_map.end()) {
            size_freed = it->second.size;
            
            // Обновление статистики
            if (stats.liveBytes >= size_freed) stats.liveBytes -= size_freed;
            stats.freedBytes += size_freed;
            
            objects_map.erase(it);
        } else {
            // Попытка удалить чужой указатель
            return; 
        }
    }

    // 2. Системное освобождение
    std::free(ptr);
    Logger::getInstance().logFree(ptr);
}

void ReferenceCountingGC::collect() {
    // Logger::getInstance().log("GC: Collection requested (No-op for RC)");
}

MemoryStats ReferenceCountingGC::getStats() const {
    std::lock_guard<std::mutex> lock(mtx);
    return stats;
}

std::string ReferenceCountingGC::name() const {
    return "ReferenceCountingGC (Map-based)";
}

void ReferenceCountingGC::addRef(void* ptr) {
    if (!ptr) return;
    std::lock_guard<std::mutex> lock(mtx);
    auto it = objects_map.find(ptr);
    if (it != objects_map.end()) {
        it->second.ref_count++;
    }
}

void ReferenceCountingGC::release(void* ptr) {
    if (!ptr) return;
    
    bool should_delete = false;
    
    {
        std::lock_guard<std::mutex> lock(mtx);
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

    // Удаляем БЕЗ блокировки mtx (так как deallocate сам возьмет блокировку)
    if (should_delete) {
        deallocate(ptr);
    }
}

} // namespace gc