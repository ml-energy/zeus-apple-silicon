#include "energy.hpp"

/* Execution time scales linearly with value of limit. */
void test_program(long long limit) {
    long long inner_limit = 10000;
    for (long long i = 0; i < limit; ++i) {
        for (long long j = 0; j < inner_limit; ++j) {}
    }
}

void print_results(const AppleEnergyMetrics& result) {
    std::cout << "CPU: " << result.cpu_mj << " mJ" << '\n';
    std::cout << "GPU: " << result.gpu_mj << " mJ" << '\n';
    std::cout << "ANE: " << result.ane_mj << " mJ" << '\n';
}

int main() {
    std::cout << "Test starting.\n";

    AppleEnergyMonitor monitor;

    monitor.begin_window("test1");
    test_program(1000);
    AppleEnergyMetrics result = monitor.end_window("test1");

    print_results(result);
}
