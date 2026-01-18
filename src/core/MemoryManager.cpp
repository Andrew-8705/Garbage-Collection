#include "config.hpp"

namespace gc {

// Глобальный статический указатель на текущую стратегию
static MemoryManager* g_active_manager = nullptr;

MemoryManager* MemoryManager::getInstance() {
    return g_active_manager;
}

void MemoryManager::setInstance(MemoryManager* gc) {
    g_active_manager = gc;
}

// Реализация printSummary
void gc::MemoryManager::printSummary() const {
    MemoryStats stats = getStats();
    
    std::cout << "\n========================================\n";
    std::cout << "   GC SUMMARY: " << name() << "\n";
    std::cout << "========================================\n";

    // Метрики объема памяти
    std::cout << "[ Memory Usage ]\n";
    std::cout << "  - Currently Live:    " << stats.liveBytes << " bytes\n";
    std::cout << "  - Total Allocated:   " << stats.allocatedBytes << " bytes\n";
    std::cout << "  - Total Freed:       " << stats.freedBytes << " bytes\n";

    std::cout << "\n[ Operations ]\n";
    std::cout << "  - Allocations Count: " << stats.totalAllocations << "\n";
    std::cout << "  - GC Cycles Run:     " << stats.totalCollections << "\n";

    // Метрики производительности (паузы GC)
    if (stats.totalCollections > 0) {
        std::cout << "\n[ Performance ]\n";
        std::printf("  - Last GC Pause:     %.4f ms\n", stats.lastCollectionTimeMs);
        std::printf("  - Total GC Time:     %.4f ms\n", stats.totalCollectionTimeMs);
        std::printf("  - Avg GC Pause:      %.4f ms\n", 
                    (stats.totalCollectionTimeMs / stats.totalCollections));
    }

    std::cout << "========================================\n" << std::endl;
}

} // namespace gc