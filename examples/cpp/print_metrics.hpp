#pragma once

#include <iostream>
#include <optional>
#include <vector>

void print_vector(const std::string& label, const std::optional<std::vector<int64_t>>& vec_opt) {
    if (vec_opt) {
        std::cout << label << ": [ ";
        for (int i = 0; i < vec_opt->size(); ++i) {
            std::cout << (*vec_opt)[i] << " mJ";
            if (i + 1 < vec_opt->size()) {
                std::cout << ", ";
            }
        }
        std::cout << " ]\n";
    }
}

void print_metric(const std::string& label, const std::optional<int64_t>& value) {
    if (value) {
        std::cout << label << ": " << *value << " mJ\n";
    }
    else {
        std::cout << label << ": unavailable\n";
    }
}

void print_apple_energy_metrics(const AppleEnergyMetrics& metrics) {
    std::cout << "--- Apple Energy Metrics ---\n";

    print_metric("CPU Total", metrics.cpu_total_mj);
    print_vector("Efficiency Cores", metrics.efficiency_cores_mj);
    print_vector("Performance Cores", metrics.performance_cores_mj);
    print_metric("Efficiency Core Manager", metrics.efficiency_core_manager_mj);
    print_metric("Performance Core Manager", metrics.performance_core_manager_mj);

    print_metric("DRAM", metrics.dram_mj);

    print_metric("GPU", metrics.gpu_mj);
    print_metric("GPU SRAM", metrics.gpu_sram_mj);

    print_metric("ANE", metrics.ane_mj);
}
