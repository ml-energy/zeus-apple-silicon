#include "energy.hpp"

/* Execution time scales linearly with value of limit. */
void test_program(long long limit) {
    volatile int x = 0;
    long long inner_limit = 10000;
    for (long long i = 0; i < limit; ++i) {
        for (long long j = 0; j < inner_limit; ++j) {
            x += j;
        }
    }
}

std::string represent_metrics(const AppleEnergyMetrics &metrics) {
    std::string repr = "";
  
    repr += "CPU Total: ";
    repr += metrics.cpu_total_mj
                ? (std::to_string(metrics.cpu_total_mj.value()) + " mJ\n")
                : "None (unavailable)\n";
  
    repr += "Efficiency cores: ";
    if (metrics.efficiency_cores_mj) {
      for (const auto &core : metrics.efficiency_cores_mj.value()) {
        repr += std::to_string(core) + " mJ  ";
      }
      repr += "\n";
    } else {
      repr += "None (unavailable)\n";
    }
  
    repr += "Performance cores: ";
    if (metrics.performance_cores_mj) {
      for (const auto &core : metrics.performance_cores_mj.value()) {
        repr += std::to_string(core) + " mJ  ";
      }
      repr += "\n";
    } else {
      repr += "None (unavailable)\n";
    }
  
    repr += "Efficiency core manager: ";
    repr += metrics.efficiency_core_manager_mj
                ? (std::to_string(metrics.efficiency_core_manager_mj.value()) +
                   " mJ\n")
                : "None (unavailable)\n";
  
    repr += "Performance core manager: ";
    repr += metrics.performance_core_manager_mj
                ? (std::to_string(metrics.performance_core_manager_mj.value()) +
                   " mJ\n")
                : "None (unavailable)\n";
  
    repr += "DRAM: ";
    repr += metrics.dram_mj ? (std::to_string(metrics.dram_mj.value()) + " mJ\n")
                            : "None (unavailable)\n";
    repr += "GPU: ";
    repr += metrics.gpu_mj ? (std::to_string(metrics.gpu_mj.value()) + " mJ\n")
                           : "None (unavailable)\n";
    repr += "GPU SRAM: ";
    repr += metrics.gpu_sram_mj
                ? (std::to_string(metrics.gpu_sram_mj.value()) + " mJ\n")
                : "None (unavailable)\n";
    repr += "ANE: ";
    repr += metrics.ane_mj ? (std::to_string(metrics.ane_mj.value()) + " mJ\n")
                           : "None (unavailable)\n";
  
    return repr;
}

int main() {
    std::cout << "Test starting...\n";

    AppleEnergyMonitor monitor;

    monitor.begin_window("test1");
    test_program(100000);
    AppleEnergyMetrics result = monitor.end_window("test1");

    std::cout << represent_metrics(result);
}
