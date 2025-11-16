#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <random>
#include <memory>
#include <chrono>

#include "abstract_container.hpp"


const int POOL_SIZE = 500;
const int CHUNK_SIZE = 200;
const int TOTAL_OPERATIONS = 1;

namespace gc {

class VectorContainer : public AbstractContainer {
private:
    std::vector<std::string> data;
public:
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) data.push_back("x" + std::to_string(i));
    }

    void shrink() override {
        if (data.size() > CHUNK_SIZE) data.resize(data.size() - CHUNK_SIZE);
    }

    void access() override {
        if (!data.empty()) const std::string& val = data[rand() % data.size()];
    }
};

class ListContainer : public AbstractContainer {
private:
    std::list<int> data;
public:
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) data.push_back(i);
    }

    void shrink() override {
        for (int i = 0; i < CHUNK_SIZE / 2 && !data.empty(); i++) data.pop_back();
    }

    void access() override {
        if (!data.empty()) {
            volatile int val = *std::next(data.begin(), rand() % data.size());
        }
    }
};

class MapContainer : public AbstractContainer {
private:
    std::map<int, std::string> data;
    int next_key = 0;
public: 
    void grow() override {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            data[next_key++] = "map_val" + std::to_string(i);
        }
    }

    void shrink() override {
        for (int i = 0; i < CHUNK_SIZE / 2 && !data.empty(); ++i) {
            data.erase(data.begin());
        }
    }

    void access() override {
        if (!data.empty()) {
            auto it = data.find(rand() % next_key);
        }
    }
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
            auto it = data.find(rand() % next_key);
        }
    }
};


class StringContainer : public AbstractContainer {
private:
    std::string data;
    const size_t APPEND_SIZE = 100;
public:
    void grow() override {
        for (int i = 0; i < 5; ++i) { 
            data += std::string(APPEND_SIZE, ' ');
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
        }
    }
};

}


int main() {
    std::srand(std::time(0));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> pool_dist(0, POOL_SIZE - 1);
    std::uniform_int_distribution<> op_dist(0, 2); // 0:grow, 1:shrink, 2:access
    std::uniform_int_distribution<> type_dist(0, 4); // 0:Vector, 1:List, 2:Map, 3:UnorderedMap, 4:String

    std::vector<std::unique_ptr<gc::AbstractContainer>> pool;
    
    // 1. Заполнение пула контейнерами
    for (int i = 0; i < POOL_SIZE; i++) {
        int type = type_dist(gen);
        switch (type) {
            case 0: pool.push_back(std::make_unique<gc::VectorContainer>());
            case 1: pool.push_back(std::make_unique<gc::ListContainer>());
            case 2: pool.push_back(std::make_unique<gc::MapContainer>());
            case 3: pool.push_back(std::make_unique<gc::UnorderedMapContainer>());
            case 4: pool.push_back(std::make_unique<gc::StringContainer>());
        }
    }
    
    std::cout << "--- STL WORKLOAD BENCHMARK ---" << std::endl;
    //std::cout << "Allocator state before workload:" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // 2. Главный цикл рандомизированных операций
    for (int i = 0; i < TOTAL_OPERATIONS; i++) {
        gc::AbstractContainer* target = pool[pool_dist(gen)].get();
        int op_type = op_dist(gen);

        if (op_type == 0) {
            target->grow();
        } else if (op_type == 1) {
            target->shrink();
        } else {
            target->access();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nWorkload finished. Total time: " << duration.count() << " ms." << std::endl;
    //std::cout << "Allocator state after workload:" << std::endl;

    return 0;
}