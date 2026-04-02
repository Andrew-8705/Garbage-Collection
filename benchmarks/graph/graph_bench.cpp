#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "../../include/core/config.hpp"
#include "../../include/utils/logger.hpp"
#include "../../include/core/gc_allocator.hpp"
#include "../../include/impl/ReferenceCountingGC.hpp"

#ifdef HAS_BDWGC
#include "../../include/impl/BoehmGCAdapter.hpp"
#endif

template <typename T>
using GCVector = std::vector<T, gc::Allocator<T>>;

#ifdef HAS_BDWGC
    #include "../../include/core/config.hpp"
    template <typename T> using GCPtr = T*;
    template <typename T, typename... Args>
    GCPtr<T> create_node(Args&&... args) { return gc::make<T>(std::forward<Args>(args)...); }
#else
    #include "../../include/core/rc_ptr.hpp"
    template <typename T> using GCPtr = gc::RcPtr<T>;
    template <typename T, typename... Args>
    GCPtr<T> create_node(Args&&... args) { return gc::make_ptr<T>(std::forward<Args>(args)...); }
#endif

// --- СТРУКТУРА ГРАФА ---
const int INITIAL_NODES = 50;
const int OPERATIONS = 500;

struct GraphNode {
    int id;
    char payload[64];
    std::vector<GCPtr<GraphNode>, gc::Allocator<GCPtr<GraphNode>>> neighbors;
    GraphNode(int _id) : id(_id) { payload[0] = (char)id; }
};

// --- БЕНЧМАРК ---
class GraphWorkload {
    //std::vector<GCPtr<GraphNode>> roots;
    GCVector<GCPtr<GraphNode>> roots; 
    std::mt19937 gen;
    int next_id = 0;

public:
    //GraphWorkload() : gen(std::random_device{}()) {}
    GraphWorkload() : gen(42) {}

    void initialize() {
        Logger::getInstance().log("Initializing Graph...");
        for (int i = 0; i < INITIAL_NODES; ++i) {
            int id = next_id++;
            roots.push_back(create_node<GraphNode>(id));
            Logger::getInstance().logGraphEvent("NODE_ADD", id);
        }
    }

    void step(int step_idx) {
        std::uniform_int_distribution<> action_dist(0, 4);
        int action = action_dist(gen);
        if (roots.empty()) action = 0;

        switch (action) {
            case 0: // ADD ROOT
            {
                int id = next_id++;
                roots.push_back(create_node<GraphNode>(id));
                Logger::getInstance().logGraphEvent("NODE_ADD", id);
                Logger::getInstance().logFormatted("Step %d: Added root node %d", step_idx, id);
                break;
            }
            case 1: // REMOVE ROOT
            {
                if (roots.empty()) break;
                std::uniform_int_distribution<> idx_dist(0, roots.size() - 1);
                int idx = idx_dist(gen);
                
                int id = -1;

                if (roots[idx]) id = roots[idx]->id;

                roots.erase(roots.begin() + idx);
                
                if (id != -1) {
                    Logger::getInstance().logGraphEvent("NODE_REM", id); // Логируем, что убрали из корней
                    Logger::getInstance().logFormatted("Step %d: Removed root node %d", step_idx, id);
                }
                break;
            }
            case 2: // ADD EDGE
            {
                if (roots.size() < 2) break;
                std::uniform_int_distribution<> idx_dist(0, roots.size() - 1);
                
                auto& from = roots[idx_dist(gen)];
                auto& to = roots[idx_dist(gen)];
                
                if (from->id == to->id) break;

                from->neighbors.push_back(to);
                Logger::getInstance().logGraphEvent("EDGE_ADD", from->id, to->id);
                Logger::getInstance().logFormatted("Step %d: Added edge %d -> %d", step_idx, from->id, to->id);
                break;
            }
            case 3: // REMOVE EDGE
            {
                if (roots.empty()) break;
                std::uniform_int_distribution<> idx_dist(0, roots.size() - 1);
                auto& node = roots[idx_dist(gen)];
                
                if (!node->neighbors.empty()) {
                    int neighbor_id = -1;
                    if (node->neighbors.back()) neighbor_id = node->neighbors.back()->id;
                    
                    node->neighbors.pop_back();
                    
                    if (neighbor_id != -1) {
                        Logger::getInstance().logGraphEvent("EDGE_REM", node->id, neighbor_id);
                        Logger::getInstance().logFormatted("Step %d: Removed edge %d -> %d", step_idx, node->id, neighbor_id);
                    }
                }
                break;
            }
            case 4: // FORCE CYCLE
            {
                int id_a = next_id++;
                int id_b = next_id++;
                
                // Создаем и логируем
                auto a = create_node<GraphNode>(id_a);
                Logger::getInstance().logGraphEvent("NODE_ADD", id_a);
                
                auto b = create_node<GraphNode>(id_b);
                Logger::getInstance().logGraphEvent("NODE_ADD", id_b);
                
                // Связываем
                a->neighbors.push_back(b);
                Logger::getInstance().logGraphEvent("EDGE_ADD", id_a, id_b);
                
                b->neighbors.push_back(a);
                Logger::getInstance().logGraphEvent("EDGE_ADD", id_b, id_a);
                
                Logger::getInstance().logFormatted("Step %d: Created cycle %d <-> %d", step_idx, id_a, id_b);
                
                Logger::getInstance().logGraphEvent("NODE_REM", id_a);
                Logger::getInstance().logGraphEvent("NODE_REM", id_b);
                break;
            }
        }
    }

    void run() {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < OPERATIONS; ++i) {
            step(i);
            //if (i % 10 == 0) {
                //gc::MemoryManager::getInstance()->collect(); 
                size_t live = gc::MemoryManager::getInstance()->getStats().liveBytes;
                Logger::getInstance().logHeapState(live);
            //}
        }

        roots.clear();
        gc::MemoryManager::getInstance()->collect();
        size_t final_live = gc::MemoryManager::getInstance()->getStats().liveBytes;
        Logger::getInstance().logHeapState(final_live);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        Logger::getInstance().log("Benchmark Finished");
        std::cout << "Time: " << duration.count() << " ms" << std::endl;
    }
};

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
    GraphWorkload bench;
    bench.initialize();
    bench.run();
    gc::MemoryManager::getInstance()->printSummary();
    return 0;
}