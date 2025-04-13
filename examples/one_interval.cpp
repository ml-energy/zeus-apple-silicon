#include "energy.hpp"

uint64_t dummy_work(uint64_t limit) {
    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < limit; i++) {
        for (uint64_t j = 0; j < 100000; j++) {
            x = i + j;
        }
    }
    return x;
}

void measure_one_interval() {
    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    dummy_work(10000);
    AppleEnergyMetrics metrics = monitor.end_window("test");

    std::cout << "CPU Energy: " << metrics.cpu_total_mj.value() << " mJ" << std::endl;

    // Note that you will likely not see significant values for GPU energy
    // unless you're exercising your GPU in some way (e.g., playing a video).
    std::cout << "GPU Energy: " << metrics.gpu_mj.value() << " mJ" << std::endl;
}

int main() {
    measure_one_interval();
}
