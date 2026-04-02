#pragma once

#include <cstddef>
#include <string>
#include <iostream>
#include <utility>


namespace gc {

    struct MemoryStats {
        size_t allocatedBytes = 0;          // количество байтов, аллоцированных с момента запуска
        size_t freedBytes = 0;              // количество байтов, освобожденных менеджером
        size_t liveBytes = 0;               // текущее количество байтов, занятых живыми объектами
        size_t totalAllocations = 0;        // общее количество вызовов 'allocate'
        size_t totalCollections = 0;        // общее количество выполненных сборок мусора
        double lastCollectionTimeMs = 0.0;  // время выполнения последней сборки мусора в миллисекундах
        double totalCollectionTimeMs = 0.0; // общее время, потраченное на сборку мусора (суммарно) в миллисекундах
        double maxPauseTimeMs = 0.0;        // максимальная пауза
        size_t peakLiveBytes = 0;           // пиковое количество "живых" байт по мнению GC
        size_t peakRSS = 0;                 // пиковое потребление памяти у ОС
    };

    class MemoryManager {
public:
    virtual ~MemoryManager() = default;

    virtual void* allocate(std::size_t size) = 0;   // аллокация памяти указанного размера
    virtual void deallocate(void* ptr) = 0;         // освобождение блока памяти
    virtual void collect() = 0;                     // принудительный запуск сборки мусора
    virtual void tick() {}                          // обновление состояния менеджера
    virtual MemoryStats getStats() const = 0;       // получение текущей статистики использования памяти
    virtual std::string name() const = 0;           // возвращение имени конкретной реализации
    virtual void printSummary() const;              // вывод сводки

    // --- Методы для Reference Counting ---
    virtual void addRef(void* ptr) {}
    virtual void release(void* ptr) {}

    // --- Глобальный доступ --- 
    static MemoryManager* getInstance();
    static void setInstance(MemoryManager* gc);

    // --- Журналирование ---
    virtual void onAllocation(std::size_t size) {}
    virtual void onDeallocation() {}
    virtual void onCollectionStart() {}
    virtual void onCollectionEnd(double durationMs) {}
};

template <typename T, typename... Args>
T* make(Args&&... args) {
    MemoryManager* gc = MemoryManager::getInstance();
    if (!gc) {
        throw std::runtime_error("GC not initialized");
    }
    
    // 1. Выделяем память через GC
    void* ptr = gc->allocate(sizeof(T));
    
    // 2. Вызываем конструктор (Placement New)
    return new(ptr) T(std::forward<Args>(args)...);
}

template <typename T>
void destroy(T* ptr) {
    if (!ptr) return;
    
    // 1. Вызываем деструктор
    ptr->~T();
    
    // 2. Освобождаем память
    MemoryManager::getInstance()->deallocate(ptr);
}
}