#include <ctime>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <functional>

#include <unistd.h>
#include <sys/time.h>
using namespace std;

int parse_args(int argc, const char *argv[], uint32_t &threads_num, uint32_t &no_accesses, uint32_t &total_bytes, uint32_t &mode) {
    int opt;
    if (argc != 9) {
        return -1;
    }
    while ((opt = getopt(argc, (char *const *)argv, "t:n:b:m:")) != -1) {
        switch (opt) {
        case 't':
            threads_num = atoi(optarg);
            assert(threads_num != 0);
            break;
        case 'n':
            no_accesses = atoi(optarg);
            break;
        case 'b':
            total_bytes = atoi(optarg);
            break;
        case 'm':
            mode = atoi(optarg);
            break;
        default:
            return -1;
        }
    }
    return 0;
}

void shuffle_pointers(void **(&pointer_array), uint32_t no_entries)
{
    uint32_t seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(pointer_array, &pointer_array[no_entries], std::default_random_engine(seed));
}

void rearrange_pointers(void **(&pointer_array), uint32_t no_entries)
{
    srand(time(0));

    /* pich a random index (x) that is greater than the current index (i) */    
    for (uint32_t i = 0; i < no_entries - 1; i++) {
        uint32_t x = rand() % (no_entries - i - 1) + 1;
        pointer_array[i] = &pointer_array[x + i];
    }
}

void do_rw_pointer_chasing(void **(&pointer_array), uint32_t no_entries, uint32_t no_accesses, uint32_t id, double *ms_timing)
{
    void **ptr = (void**) pointer_array;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < no_accesses; i++) {
        void **tmp = ptr;
        *ptr = *(void**) ((uint64_t)*tmp & 0xffffffffffffffff);
        ptr = (void**) *ptr;
    }
    auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double, milli> timing = end - start;
    ms_timing[id] = timing.count();
}

void do_read_pointer_chasing(void **(&pointer_array), uint32_t no_entries, uint32_t no_accesses, uint32_t id, double *ms_timing)
{
    void **ptr = (void**) pointer_array;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < no_accesses; i++) {
        ptr = (void **)*ptr;
    }
    auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double, milli> timing = end - start;

    ms_timing[id] = timing.count();
}

int main(int argc, const char * argv[]) {
    void **pointer_array;
    double *ms_timing, latency = 0;
    uint32_t no_entries, threads_num, no_accesses, total_bytes, mode;
    function<void(void **&, uint32_t, uint32_t, uint32_t, double *)> pointer_chasing_fun;

    if (parse_args(argc, argv, threads_num, no_accesses, total_bytes, mode)) {
        cerr << "Usage: %s [-t threads_num] [-n number_of_accesses] [-b total_bytes] [-m mode (0 for read only, 1 for read/write)]\n" << argv[0] << endl;
        return -1;
    }
    pointer_chasing_fun = mode ? do_rw_pointer_chasing : do_read_pointer_chasing; 
    no_entries = total_bytes / sizeof(void*);

    ms_timing = new double[threads_num];
    pointer_array = new void*[no_entries];

    pointer_array[no_entries - 1] = &pointer_array[0];
    for (uint32_t i = 0; i < no_entries - 1; i++) {
        pointer_array[i] = &pointer_array[i+1];
    }

    shuffle_pointers(pointer_array, no_entries);

    vector<thread> threads;
    for (uint32_t i = 0; i < threads_num; i++) {
        threads.push_back(std::thread(pointer_chasing_fun, ref(pointer_array), no_entries, no_accesses, i, ref(ms_timing)));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    for (uint32_t i = 0; i < threads_num; i++) {
        latency += ms_timing[i];
    }

    // cout << "Average latency for " << threads_num << " thread(s): " << latency / threads_num << "ms" << endl;
    cout << latency / threads_num << endl;

    delete[] ms_timing;
    delete[] pointer_array;
    return 0;
}