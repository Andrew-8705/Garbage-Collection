#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <random>
#include <memory>
#include <chrono>

#include "abstract_container.hpp"
#include "gc_allocator.hpp"
#include "logger.hpp"

#include "ReferenceCountingGC.hpp"

#ifdef HAS_BDWGC
#include "BoehmGCAdapter.hpp"
#endif


const int POOL_SIZE = 500;       // сколько контейнеров будет создано
const int CHUNK_SIZE = 200;      // сколько элементов будет добавлено в контейнер
const int TOTAL_OPERATIONS = 50; // сколько операций для выбранного контейнера будет выполнено

namespace gc {

using GCString = std::basic_string<char, std::char_traits<char>, gc::Allocator<char>>;

template <typename T>
using GCVector = std::vector<T, gc::Allocator<T>>;

template <typename T>
using GCList = std::list<T, gc::Allocator<T>>;

template <typename K, typename V>
using GCMap = std::map<K, V, std::less<K>, gc::Allocator<std::pair<const K, V>>>;

template <typename K, typename V>
using GCUnorderedMap = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, gc::Allocator<std::pair<const K, V>>>;

// ---------------------------------------------------------

class VectorContainer : public AbstractContainer {
private:
    GCVector<GCString> data;
public:
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            data.push_back(GCString("x") + GCString(std::to_string(i).c_str()));
        }
    }

    void shrink() override {
        if (data.size() > CHUNK_SIZE) data.resize(data.size() - CHUNK_SIZE);
    }

    void access() override {
        if (!data.empty()) {
            const GCString& val = data[rand() % data.size()];
            (void)val;
        }
    }

    const char* getName() const override { return "VectorContainer"; }
};

class ListContainer : public AbstractContainer {
private:
    GCList<int> data;
public:
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) 
            data.push_back(i);
    }

    void shrink() override {
        for (int i = 0; i < CHUNK_SIZE / 2 && !data.empty(); i++) 
            data.pop_back();
    }

    void access() override {
        if (!data.empty()) {
            volatile int val = *std::next(data.begin(), rand() % data.size());
            (void)val;
        }
    }

    const char* getName() const override { return "ListContainer"; }
};

class MapContainer : public AbstractContainer {
private:
    std::map<int, GCString> data;
    int next_key = 0;
public: 
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            data[next_key++] = GCString("map_val") + GCString(std::to_string(i).c_str());
        }
    }

    void shrink() override {
        for (int i = 0; i < CHUNK_SIZE / 2 && !data.empty(); ++i) {
            data.erase(data.begin());
        }
    }

    void access() override {
        if (!data.empty()) {
            if (next_key > 0) {
                 auto it = data.find(rand() % next_key);
                 (void)it;
            }
        }
    }

    const char* getName() const override { return "MapContainer"; }
};

class UnorderedMapContainer : public AbstractContainer {
private:
    std::unordered_map<int, int> data;
    int next_key = 0;
public:
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            data[next_key++] = i;
        }
    }

    void shrink() override {
        for (int i = 0; i < CHUNK_SIZE / 2 && !data.empty(); ++i) {
            data.erase(data.begin());
        }
    }

    void access() override {
        if (!data.empty()) {
             if (next_key > 0) {
                auto it = data.find(rand() % next_key);
                (void)it;
             }
        }
    }

    const char* getName() const override { return "UnorderedMapContainer"; }
};


class StringContainer : public AbstractContainer {
private:
    GCString data;
    const size_t APPEND_SIZE = 100;
public:
    void grow() override {
        for (int i = 0; i < 5; ++i) { 
            data += GCString(APPEND_SIZE, ' ');
        }
    }

    void shrink() override {
        if (data.size() > APPEND_SIZE) {
            data.resize(data.size() - APPEND_SIZE);
        } else {
            data.clear();
        }
    }

    void access() override {
        if (!data.empty()) {
            char c = data[0];
            (void)c;
        }
    }

    const char* getName() const override { return "StringContainer"; }
};

}


int main() {
    #ifdef HAS_BDWGC
        static gc::BoehmGCAdapter gc_instance;
        std::cout << "Strategy: Boehm-Demers-Weiser GC\n";
    #else
        static gc::ReferenceCountingGC gc_instance;
        std::cout << "Strategy: Reference Counting\n";
    #endif

    gc::MemoryManager::setInstance(&gc_instance);


    Logger::getInstance();
    // std::srand(std::time(0));
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::srand(42);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> pool_dist(0, POOL_SIZE - 1);
    std::uniform_int_distribution<> op_dist(0, 2); // 0:grow, 1:shrink, 2:access
    std::uniform_int_distribution<> type_dist(0, 4); // 0:Vector, 1:List, 2:Map, 3:UnorderedMap, 4:String

    std::vector<std::unique_ptr<gc::AbstractContainer>> pool;
    
    // 1. Заполнение пула контейнерами
    for (int i = 0; i < POOL_SIZE; i++) {
        int type = type_dist(gen);
        switch (type) {
            case 0: { 
                pool.push_back(std::make_unique<gc::VectorContainer>());
                Logger::getInstance().log("Created: VectorContainer"); 
                break;
            }
            case 1: {
                pool.push_back(std::make_unique<gc::ListContainer>());
                Logger::getInstance().log("Created: ListContainer");
                break;
            }
            case 2: {
                pool.push_back(std::make_unique<gc::MapContainer>());
                Logger::getInstance().log("Created: MapContainer");
                break;
            }
            case 3: {
                pool.push_back(std::make_unique<gc::UnorderedMapContainer>());
                Logger::getInstance().log("Created: UnorderedMapContainer");
                break;
            }
            case 4: {
                pool.push_back(std::make_unique<gc::StringContainer>());
                Logger::getInstance().log("Created: StringContainer");
                break;
            }
            default: break;
        }
    }
    
    std::cout << "--- STL WORKLOAD BENCHMARK ---" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // 2. Главный цикл рандомизированных операций
    for (int i = 0; i < TOTAL_OPERATIONS; i++) {
        int target_idx = pool_dist(gen);
        gc::AbstractContainer* target = pool[target_idx].get();
        int op_type = op_dist(gen);
        
        const char* op_name = "";

        if (op_type == 0) {
            op_name = "GROW";
            target->grow();
        } else if (op_type == 1) {
            op_name = "SHRINK";
            target->shrink();
        } else {
            op_name = "ACCESS";
            target->access();
        }

        Logger::getInstance().logFormatted("Op #%d: [%s] %s at index %d",  i, target->getName(), op_name, target_idx);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nWorkload finished. Total time: " << duration.count() << " ms." << std::endl;
    gc::MemoryManager::getInstance()->printSummary();

    return 0;
}