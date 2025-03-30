#include "actually_smart_pointer.hpp"

#include <string>
#include <chrono>
#include <fstream>
#include <vector>
#include <sys/resource.h>

using namespace std::chrono;

size_t current_rss_kb() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
#if defined(__APPLE__) && defined(__MACH__)
    return usage.ru_maxrss / 1024;
#else
    return usage.ru_maxrss;
#endif
}

template <typename Fn>
std::pair<double, size_t> measure(Fn&& fn) {
    auto mem_before = current_rss_kb();
    auto start = high_resolution_clock::now();

    fn();

    auto end = high_resolution_clock::now();
    auto mem_after = current_rss_kb();
    double time_ms = duration<double, std::milli>(end - start).count();
    size_t mem_used_kb = (mem_after > mem_before) ? (mem_after - mem_before) : 0;
    return {time_ms, mem_used_kb};
}

int main() {
    std::ofstream out("benchmark_asp.csv");
    out << "iterations,time_ms,memory_b\n";

    // spare pointer so llama will be initialized;
    {
        asp::actually_smart_pointer<std::string> p(new std::string("test"));
    }

    for (int count = 10; count <= 100; count += 5) {
        auto [time, mem] = measure([&] {
            for (int i = 0; i < count; ++i) {
                asp::actually_smart_pointer<std::string> p(new std::string("test"));
            }
        });
        out << count << "," << time << "," << mem << "\n";
        out.flush();
    }

    return 0;
}

