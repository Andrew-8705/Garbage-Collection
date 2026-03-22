#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

#include "rocksdb/db.h"
#include "rocksdb/options.h"

namespace custom_allocator {
    void print_stats() {
        std::cout << "+";
    }
}

using namespace rocksdb;
using namespace std;

string random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    string str(length, 0);
    generate_n(str.begin(), length, randchar);
    return str;
}

void run_thread_workload(const string& db_path, int num_ops, int thread_id) {
    DB* db = nullptr;
    Options options;
    options.create_if_missing = true;
    options.OptimizeLevelStyleCompaction();

    options.write_buffer_size = 4 * 1024 * 1024;

    Status s = DB::Open(options, db_path, &db);
    if (!s.ok()) {
        cerr << "Thread " << thread_id << " failed to open DB: " << s.ToString() << '\n';
        return;
    }

    string value_size_500 = random_string(500);


    for (int i = 0; i < num_ops; ++i) {
        string key = "key-" + to_string(thread_id) + "-" + to_string(i); 
        s = db->Put(WriteOptions(), key, value_size_500);
    }

    string value;
    for (int i = 0; i < num_ops; ++i) {
        string key = "key-" + to_string(thread_id) + "-" + to_string(i);
        s = db->Get(ReadOptions(), key, &value);
    }

    delete db; 
    
    Status ds = DestroyDB(db_path, options);
    if (!ds.ok()) {
         cerr << "Thread " << thread_id << " failed to destroy DB: " << ds.ToString() << '\n';
    }
}


int main() {
    srand(time(0));
    const int NUM_THREADS = 4;
    const int OPS_PER_THREAD = 100000;
    const string DB_BASE_PATH = "/tmp/rocksdb_bench_";

    cout << "--- ROCKSDB WORKLOAD BENCHMARK ---" << '\n';

    cout << "Allocator state before workload:" << '\n';
    custom_allocator::print_stats(); 

    vector<thread> threads;
    auto start = chrono::high_resolution_clock::now();

    cout << "Starting workload with " << NUM_THREADS << " threads (" 
         << OPS_PER_THREAD * NUM_THREADS << " total operations)..." << '\n';
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        string db_path = DB_BASE_PATH + to_string(i);
        threads.emplace_back(run_thread_workload, db_path, OPS_PER_THREAD, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "\nWorkload finished. Total time: " << duration.count() << " ms." << '\n';
    
    cout << "Allocator state after workload:" << '\n';
    custom_allocator::print_stats();

    return 0;
}