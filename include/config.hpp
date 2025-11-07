#include <cstddef>
#include <string>
#include <new>
#include <iostream>


namespace gc {

    struct MemoryStats {
        size_t allocatedBytes = 0; // количество байтов, аллоцированных с момента запуска
        size_t freedBytes = 0; // количество байтов, освобожденных менеджером
        size_t liveBytes = 0; // текущее количество байтов, занятых живыми объектами
        size_t totalAllocations = 0; // общее количество вызовов 'allocate'
        size_t totalCollections = 0; // общее количество выполненных сборок мусора
        double lastCollectionTimeMs = 0.0; // время выполнения последней сборки мусора в миллисекундах
        double totalCollectionTimeMs = 0.0; // общее время, потраченное на сборку мусора (суммарно) в миллисекундах
    };

    class MemoryManager {
public:
    virtual ~MemoryManager() = default;

    virtual void* allocate(std::size_t size) = 0; // аллокация памяти указанного размера
    virtual void deallocate(void* ptr) = 0; // освобождение блока памяти
    virtual void collect() = 0; // принудительный запуск сборки мусора
    virtual void tick() {} // обновление состояния менеджера
    virtual MemoryStats getStats() const = 0; // получение текущей статистики использования памяти
    virtual std::string name() const = 0; // возвращение имени конкретной реализации
    virtual void printSummary() const; // вывод сводки

    // --- Журналирование ---
    virtual void onAllocation(std::size_t size) {}
    virtual void onDeallocation() {}
    virtual void onCollectionStart() {}
    virtual void onCollectionEnd(double durationMs) {}
};
}