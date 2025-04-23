#include <iostream>
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

void overlapping_intervals()
{
    AppleEnergyMonitor monitor;

    std::cout << "[ Partially overlapping measurement windows ]\n";
    monitor.begin_window("window1");
    monitor.begin_window("window2");
    dummy_work(1000);
    AppleEnergyMetrics metrics1 = monitor.end_window("window1");
    AppleEnergyMetrics metrics2 = monitor.end_window("window2");

    print_apple_energy_metrics(metrics1);
    print_apple_energy_metrics(metrics2);

    std::cout << '\n';

    std::cout << "[ One measurement window completely surrounds another ]\n";
    // Note: once a window ends, its name is free to be reused.
    monitor.begin_window("window1");
    monitor.begin_window("window2");
    dummy_work(1000);
    metrics2 = monitor.end_window("window2");
    metrics1 = monitor.end_window("window1");

    print_apple_energy_metrics(metrics1);
    print_apple_energy_metrics(metrics2);
}

int main()
{
    overlapping_intervals();
}
