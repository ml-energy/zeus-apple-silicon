#include "apple_energy.hpp"
#include "print_metrics.hpp"

uint64_t dummy_work(uint64_t limit)
{
    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < limit; i++) {
        for (uint64_t j = 0; j < 100000; j++) {
            x = i + j;
        }
    }
    return x;
}

void one_interval()
{
    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    dummy_work(1000);
    AppleEnergyMetrics metrics = monitor.end_window("test");

    print_apple_energy_metrics(metrics);
}

int main()
{
    one_interval();
}
