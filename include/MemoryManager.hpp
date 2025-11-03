/**
 * @file IMemoryManger
 * @brief Определяет интерфейс для менеджеров памяти и сборщиков мусора.
 */
#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <chrono>

namespace gcbench {
/**
 * @struct MemoryStats
 * @brief Структура, содержащая ключевые метрики использования памяти и GC.
 * * *Используется для диагностики и мониторинга производительности.*
 */
struct MemoryStats {
    /** Количество байтов, аллоцированных с момента запуска. */
    size_t allocatedBytes = 0;
    /** Количество байтов, освобожденных менеджером. */
    size_t freedBytes = 0;
    /** Текущее количество байтов, занятых живыми объектами. */
    size_t liveBytes = 0;
    /** Общее количество вызовов 'allocate'. */
    size_t totalAllocations = 0;
    /** Общее количество выполненных сборок мусора. */
    size_t totalCollections = 0;
    /** Время выполнения последней сборки мусора в миллисекундах. */
    double lastCollectionTimeMs = 0.0;
    /** Общее время, потраченное на сборку мусора (суммарно) в миллисекундах. */
    double totalCollectionTimeMs = 0.0;
};

/**
 * @class MemoryManager
 * @brief Абстрактный интерфейс для всех пользовательских менеджеров памяти/GC.
 *
 * *Этот интерфейс определяет минимальный набор функций, 
 * необходимых для управления памятью в тестовой среде.* * Все реализации GC должны наследовать этот класс.
 */
class MemoryManager {
public:
    virtual ~MemoryManager() = default;

    // --- Основные операции ---
    /**
     * @brief Аллокация блока памяти указанного размера.
     * @param size Требуемый размер блока в байтах.
     * @return void* Указатель на выделенный блок памяти.
     * @throw std::bad_alloc Если память не может быть выделена.
     * @pre size > 0.
     */
    virtual void* allocate(std::size_t size) = 0;

    /**
     * @brief Освобождение блока памяти.
     * * *Для традиционных GC этот метод может быть no-op, так как освобождение
     * происходит автоматически во время collect().*
     * @param ptr Указатель на блок, который нужно освободить (может быть nullptr).
     */
    virtual void deallocate(void* ptr) = 0;

    /**
     * @brief Принудительный запуск сборки мусора.
     * @details Может быть синхронным или асинхронным, зависит от реализации.
     */
    virtual void collect() = 0;

    /**
     * @brief Обновление состояния менеджера (например, шаг инкрементального GC).
     * @details Вызывается тестовым циклом для продвижения внутреннего состояния. 
     * По умолчанию пустая операция.
     */
    virtual void tick() {}

    // --- Метрики ---
    /**
     * @brief Получает текущую статистику использования памяти.
     * @return MemoryStats Структура с метриками.
     * @see MemoryStats
     */
    virtual MemoryStats getStats() const = 0;

    // --- Диагностика и конфигурация ---
    /**
     * @brief Возвращает имя конкретной реализации менеджера.
     * @return std::string Имя менеджера (например, "Mark and Sweep GC").
     */
    virtual std::string name() const = 0;

    /**
     * @brief Выводит краткую сводку по работе менеджера в stdout/лог.
     */
    virtual void printSummary() const;

    // --- Журналирование ---
    virtual void onAllocation(std::size_t size) {}
    virtual void onDeallocation() {}
    virtual void onCollectionStart() {}
    virtual void onCollectionEnd(double durationMs) {}
};

} // namespace gcbench
