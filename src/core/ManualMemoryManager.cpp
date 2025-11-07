// /**
//  * @file ManualMemoryManager.cpp
//  * @brief Реализация MemoryManager, использующая стандартные операторы new/delete.
//  */

// #include "../../include/MemoryManager.hpp"
// #include <new>
// #include <iostream>

// namespace gcbench {

// /**
//  * @class ManualMemoryManager
//  * @brief Менеджер памяти, реализующий ручное управление через `::operator new` и `::operator delete`.
//  *
//  * *Этот класс не выполняет автоматическую сборку мусора (GC). 
//  * Пользователь несет ответственность за явный вызов `delete` или `deallocate` 
//  * для освобождения памяти. *Метод `collect()` является пустым (no-op).
//  */
// class ManualMemoryManager : public MemoryManager {
//     /** * @var stats
//      * @brief Статистика использования памяти для ручного менеджера.
//      */
//     MemoryStats stats;

// public:
// /**
//      * @brief Выделяет блок памяти, используя глобальный `::operator new`.
//      * @param size Требуемый размер блока в байтах.
//      * @return void* Указатель на выделенный блок памяти.
//      * @details Обновляет статистику: `allocatedBytes`, `liveBytes`, `totalAllocations`.
//      */
//     void* allocate(std::size_t size) override {
//         void* ptr = ::operator new(size);
//         stats.allocatedBytes += size;
//         stats.liveBytes += size;
//         stats.totalAllocations++;
//         onAllocation(size);
//         return ptr;
//     }

//     /**
//      * @brief Освобождает блок памяти, используя глобальный `::operator delete`.
//      * @param ptr Указатель на блок, который нужно освободить.
//      * @details В данной реализации статистика `freedBytes` не обновляется, 
//      * поскольку размер блока неизвестен. Вызывает хук `onDeallocation()`.
//      * @todo В реальном приложении нужно хранить размер для корректного обновления статистики.
//      */
//     void deallocate(void* ptr) override {
//         // Тут в реальности нужно знать размер (можно хранить в карте)
//         onDeallocation();
//         ::operator delete(ptr);
//     }

//     /**
//      * @brief Запуск сборки мусора (фиктивная операция).
//      * @details В этом менеджере нет автоматической сборки мусора, поэтому метод не выполняет никаких действий (no-op).
//      * @see MemoryManager::collect()
//      */
//     void collect() override {
//         // Нет автоматического GC — просто фиктивный вызов
//     }

//     /**
//      * @brief Возвращает накопленную статистику использования памяти.
//      * @return MemoryStats Текущая статистика.
//      */
//     MemoryStats getStats() const override {
//         return stats;
//     }

//     /**
//      * @brief Возвращает имя менеджера.
//      * @return std::string Строка "Manual (new/delete)".
//      */
//     std::string name() const override { return "Manual (new/delete)"; }
// };

// }
