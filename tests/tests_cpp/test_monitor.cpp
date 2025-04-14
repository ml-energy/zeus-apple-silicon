#include "apple_energy.hpp"
#include "ioreport_mocker.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::string represent_metrics(const AppleEnergyMetrics& metrics);

void test_one_interval()
{
    std::cout << "* Running test_one_interval\n";
    Mocker mocker;

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "CPU Energy", { 10000000, "mJ" } },
        { "GPU Energy", { 1000000, "nJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "CPU Energy", { 55000000, "mJ" } },
        { "GPU Energy", { 3000000, "nJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    assert(result.cpu_total_mj.value() == 45000000);
    assert(result.gpu_mj.value() == 2);

    std::cout << "  > test_one_interval passed.\n";
}

void test_unit_handling()
{
    std::cout << "* Running test_unit_handling\n";
    Mocker mocker;

    // --- Test with mJ ---
    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "CPU Energy", { 10000000, "mJ" } },
        { "GPU Energy", { 1000000, "mJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "CPU Energy", { 55000000, "mJ" } },
        { "GPU Energy", { 3000000, "mJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics res1 = monitor.end_window("test");

    assert(res1.cpu_total_mj.value() == 45000000);
    assert(res1.gpu_mj.value() == 2000000);

    // --- Test with J ---
    data1 = {
        { "CPU Energy", { 10000000, "J" } },
        { "GPU Energy", { 1000000, "J" } },
    };
    mocker.push_back_sample(data1);

    data2 = {
        { "CPU Energy", { 55000000, "J" } },
        { "GPU Energy", { 3000000, "J" } },
    };
    mocker.push_back_sample(data2);

    monitor.begin_window("test");
    AppleEnergyMetrics res2 = monitor.end_window("test");

    assert(res2.cpu_total_mj.value() == 45'000'000'000LL);
    assert(res2.gpu_mj.value() == 2'000'000'000LL);

    // --- Test with differing units ---
    data1 = {
        { "CPU Energy", { 10000000, "J" } },
        { "GPU Energy", { 1000000, "nJ" } },
    };
    mocker.push_back_sample(data1);

    data2 = {
        { "CPU Energy", { 55000000, "kJ" } },
        { "GPU Energy", { 3000000, "J" } },
    };
    mocker.push_back_sample(data2);

    monitor.begin_window("test");
    AppleEnergyMetrics res3 = monitor.end_window("test");

    assert(res3.cpu_total_mj.value() == 54'990'000'000'000LL);
    assert(res3.gpu_mj.value() == 2'999'999'999LL);

    // --- Test differing units accumulating to same fields ---
    data1 = {
        { "P0CPM", { 2000, "J" } },
        { "P1CPM", { 2000, "mJ" } },

        { "ECPU0", { 20, "kJ" } },
        { "ECPU1", { 20, "J" } },
        { "ECPU2", { 20, "mJ" } },
        { "ECPU3", { 2000000, "nJ" } }
    };
    mocker.push_back_sample(data1);
    data1 = {
        { "P0CPM", { 3000, "J" } },
        { "P1CPM", { 7000, "mJ" } },

        { "ECPU0", { 40, "kJ" } },
        { "ECPU1", { 40, "J" } },
        { "ECPU2", { 40, "mJ" } },
        { "ECPU3", { 4000000, "nJ" } }
    };
    mocker.push_back_sample(data1);

    monitor.begin_window("test");
    AppleEnergyMetrics res4 = monitor.end_window("test");

    // 1000000 + 20000000 + 5000 mj
    assert(res4.performance_core_manager_mj.value() == 1005000);

    std::vector<int64_t> ecore_expected = { 20000000, 20000, 20, 2 };
    std::vector<int64_t> ecore_result = res4.efficiency_cores_mj.value();
    std::sort(ecore_result.begin(), ecore_result.end(), std::greater<int64_t>());
    assert(ecore_result == ecore_expected);

    std::cout << "  > test_unit_handling passed.\n";
}


std::string represent_metrics(const AppleEnergyMetrics& metrics)
{
    std::string repr = "";

    repr += "CPU Total: ";
    repr += metrics.cpu_total_mj
        ? (std::to_string(metrics.cpu_total_mj.value()) + " mJ\n")
        : "None (unavailable)\n";

    repr += "Efficiency cores: ";
    if (metrics.efficiency_cores_mj) {
        for (const auto& core : metrics.efficiency_cores_mj.value()) {
            repr += std::to_string(core) + " mJ  ";
        }
        repr += "\n";
    } else {
        repr += "None (unavailable)\n";
    }

    repr += "Performance cores: ";
    if (metrics.performance_cores_mj) {
        for (const auto& core : metrics.performance_cores_mj.value()) {
            repr += std::to_string(core) + " mJ  ";
        }
        repr += "\n";
    } else {
        repr += "None (unavailable)\n";
    }

    repr += "Efficiency core manager: ";
    repr += metrics.efficiency_core_manager_mj
        ? (std::to_string(metrics.efficiency_core_manager_mj.value()) + " mJ\n")
        : "None (unavailable)\n";

    repr += "Performance core manager: ";
    repr += metrics.performance_core_manager_mj
        ? (std::to_string(metrics.performance_core_manager_mj.value()) + " mJ\n")
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

int main()
{
    std::cout << "--- Starting tests ---\n";

    test_one_interval();
    test_unit_handling();

    std::cout << "--- All tests passed --- \n";
}
