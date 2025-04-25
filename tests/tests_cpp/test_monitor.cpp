#include "apple_energy.hpp"
#include "ioreport_mocker.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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

void compare_results(const AppleEnergyMetrics& result, const AppleEnergyMetrics& expected)
{
    assert(result.cpu_total_mj == expected.cpu_total_mj);
    assert(result.efficiency_core_manager_mj == expected.efficiency_core_manager_mj);
    assert(result.performance_core_manager_mj == expected.performance_core_manager_mj);
    assert(result.dram_mj == expected.dram_mj);
    assert(result.gpu_mj == expected.gpu_mj);
    assert(result.gpu_sram_mj == expected.gpu_sram_mj);
    assert(result.ane_mj == expected.ane_mj);

    if (expected.efficiency_cores_mj) {
        assert(result.efficiency_cores_mj);

        std::vector<int64_t> ecore_expected = expected.efficiency_cores_mj.value();
        std::vector<int64_t> ecore_result = result.efficiency_cores_mj.value();

        std::sort(ecore_expected.begin(), ecore_expected.end());
        std::sort(ecore_result.begin(), ecore_result.end());

        assert(ecore_expected == ecore_result);
    } else {
        assert(!result.efficiency_cores_mj);
    }

    if (expected.performance_cores_mj) {
        assert(result.performance_cores_mj);

        std::vector<int64_t> pcore_expected = expected.performance_cores_mj.value();
        std::vector<int64_t> pcore_result = result.performance_cores_mj.value();

        std::sort(pcore_expected.begin(), pcore_expected.end());
        std::sort(pcore_result.begin(), pcore_result.end());

        assert(pcore_expected == pcore_result);
    } else {
        assert(!result.performance_cores_mj);
    }
}

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

void test_m1_max_example()
{
    std::cout << "* Running test_m1_max_example\n";
    Mocker mocker;

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "EACC_CPU0", { 0, "mJ" } },
        { "EACC_CPU1", { 0, "mJ" } },

        { "PACC0_CPU0", { 0, "mJ" } },
        { "PACC0_CPU1", { 0, "mJ" } },
        { "PACC0_CPU2", { 0, "mJ" } },
        { "PACC0_CPU3", { 0, "mJ" } },

        { "PACC1_CPU0", { 0, "mJ" } },
        { "PACC1_CPU1", { 0, "mJ" } },
        { "PACC1_CPU2", { 0, "mJ" } },
        { "PACC1_CPU3", { 0, "mJ" } },

        { "EACC_CPM", { 0, "mJ" } },
        { "PACC0_CPM", { 0, "mJ" } },
        { "PACC1_CPM", { 0, "mJ" } },

        { "CPU Energy", { 0, "mJ" } },
        { "GPU SRAM0", { 0, "mJ" } },
        { "ANE0", { 0, "mJ" } },
        { "DRAM0", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "EACC_CPU0", { 4, "mJ" } },
        { "EACC_CPU1", { 2, "mJ" } },

        { "PACC0_CPU0", { 1651, "mJ" } },
        { "PACC0_CPU1", { 14, "mJ" } },
        { "PACC0_CPU2", { 2, "mJ" } },
        { "PACC0_CPU3", { 0, "mJ" } },

        { "PACC1_CPU0", { 0, "mJ" } },
        { "PACC1_CPU1", { 0, "mJ" } },
        { "PACC1_CPU2", { 0, "mJ" } },
        { "PACC1_CPU3", { 0, "mJ" } },

        { "EACC_CPM", { 2, "mJ" } },
        { "PACC0_CPM", { 20, "mJ" } },
        { "PACC1_CPM", { 6, "mJ" } },

        { "CPU Energy", { 1701, "mJ" } },
        { "GPU SRAM0", { 0, "mJ" } },
        { "ANE0", { 0, "mJ" } },
        { "DRAM0", { 358, "mJ" } },
        { "GPU Energy", { 9104980, "nJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;
    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    AppleEnergyMetrics expected = {
        std::make_optional(1701),
        std::make_optional(std::vector<int64_t> { 4, 2 }),
        std::make_optional(std::vector<int64_t> { 1651, 14, 2, 0, 0, 0, 0, 0 }),
        std::make_optional(2),
        std::make_optional(26),
        std::make_optional(358),
        std::make_optional(9),
        std::make_optional(0),
        std::make_optional(0)
    };

    compare_results(result, expected);

    std::cout << "  > test_m1_max_example passed.\n";
}

void test_m3_pro_example()
{
    std::cout << "* Running test_m3_pro_example\n";
    Mocker mocker;

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "ECPU0", { 0, "mJ" } },
        { "ECPU1", { 0, "mJ" } },
        { "ECPU2", { 0, "mJ" } },
        { "ECPU3", { 0, "mJ" } },
        { "ECPU4", { 0, "mJ" } },
        { "ECPU5", { 0, "mJ" } },

        { "PCPU0", { 0, "mJ" } },
        { "PCPU1", { 0, "mJ" } },
        { "PCPU2", { 0, "mJ" } },
        { "PCPU3", { 0, "mJ" } },
        { "PCPU4", { 0, "mJ" } },
        { "PCPU5", { 0, "mJ" } },

        { "ECPM", { 0, "mJ" } },
        { "PCPM", { 0, "mJ" } },

        { "CPU Energy", { 0, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "ECPU0", { 5, "mJ" } },
        { "ECPU1", { 2, "mJ" } },
        { "ECPU2", { 2, "mJ" } },
        { "ECPU3", { 1, "mJ" } },
        { "ECPU4", { 1, "mJ" } },
        { "ECPU5", { 2, "mJ" } },

        { "PCPU0", { 1, "mJ" } },
        { "PCPU1", { 0, "mJ" } },
        { "PCPU2", { 9, "mJ" } },
        { "PCPU3", { 0, "mJ" } },
        { "PCPU4", { 3, "mJ" } },
        { "PCPU5", { 1893, "mJ" } },

        { "ECPM", { 5, "mJ" } },
        { "PCPM", { 19, "mJ" } },

        { "CPU Energy", { 2131, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 60, "mJ" } },
        { "GPU Energy", { 4126124, "nJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    AppleEnergyMetrics expected = {
        std::make_optional(2131),
        std::make_optional(std::vector<int64_t> { 5, 2, 2, 1, 1, 2 }),
        std::make_optional(std::vector<int64_t> { 1, 0, 9, 0, 3, 1893 }),
        std::make_optional(5),
        std::make_optional(19),
        std::make_optional(60),
        std::make_optional(4),
        std::make_optional(0),
        std::make_optional(0)
    };

    compare_results(result, expected);

    std::cout << "  > test_m3_pro_example passed.\n";
}

void test_m4_example()
{
    std::cout << "* Running test_m4_example\n";
    Mocker mocker;

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "ECPU0", { 0, "mJ" } },
        { "ECPU1", { 0, "mJ" } },
        { "ECPU2", { 0, "mJ" } },
        { "ECPU3", { 0, "mJ" } },
        { "ECPU4", { 0, "mJ" } },
        { "ECPU5", { 0, "mJ" } },

        { "PCPU0", { 0, "mJ" } },
        { "PCPU1", { 0, "mJ" } },
        { "PCPU2", { 0, "mJ" } },
        { "PCPU3", { 0, "mJ" } },

        { "ECPM", { 0, "mJ" } },
        { "PCPM", { 0, "mJ" } },

        { "CPU Energy", { 0, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "ECPU0", { 2, "mJ" } },
        { "ECPU1", { 1, "mJ" } },
        { "ECPU2", { 0, "mJ" } },
        { "ECPU3", { 0, "mJ" } },
        { "ECPU4", { 0, "mJ" } },
        { "ECPU5", { 0, "mJ" } },

        { "PCPU0", { 47, "mJ" } },
        { "PCPU1", { 1, "mJ" } },
        { "PCPU2", { 2104, "mJ" } },
        { "PCPU3", { 3, "mJ" } },

        { "ECPM", { 2, "mJ" } },
        { "PCPM", { 16, "mJ" } },

        { "CPU Energy", { 2174, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 61, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    AppleEnergyMetrics expected = {
        std::make_optional(2174),
        std::make_optional(std::vector<int64_t> { 2, 1, 0, 0, 0, 0 }),
        std::make_optional(std::vector<int64_t> { 47, 1, 2104, 3 }),
        std::make_optional(2),
        std::make_optional(16),
        std::make_optional(61),
        std::make_optional(0),
        std::make_optional(0),
        std::make_optional(0)
    };

    compare_results(result, expected);

    std::cout << "  > test_m4_example passed.\n";
}

void test_m4_pro_example()
{
    std::cout << "* Running test_m4_pro_example\n";
    Mocker mocker;

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        { "EACC_CPU0", { 0, "mJ" } },
        { "EACC_CPU1", { 0, "mJ" } },
        { "EACC_CPU2", { 0, "mJ" } },
        { "EACC_CPU3", { 0, "mJ" } },

        { "PACC0_CPU0", { 0, "mJ" } },
        { "PACC0_CPU1", { 0, "mJ" } },
        { "PACC0_CPU2", { 0, "mJ" } },
        { "PACC0_CPU3", { 0, "mJ" } },
        { "PACC0_CPU4", { 0, "mJ" } },
        { "PACC0_CPU5", { 0, "mJ" } },

        { "PACC1_CPU0", { 0, "mJ" } },
        { "PACC1_CPU1", { 0, "mJ" } },
        { "PACC1_CPU2", { 0, "mJ" } },
        { "PACC1_CPU3", { 0, "mJ" } },
        { "PACC1_CPU4", { 0, "mJ" } },
        { "PACC1_CPU5", { 0, "mJ" } },

        { "EACC_CPM", { 0, "mJ" } },
        { "PACC0_CPM", { 0, "mJ" } },
        { "PACC1_CPM", { 0, "mJ" } },

        { "CPU Energy", { 0, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "EACC_CPU0", { 1, "mJ" } },
        { "EACC_CPU1", { 1, "mJ" } },
        { "EACC_CPU2", { 0, "mJ" } },
        { "EACC_CPU3", { 0, "mJ" } },

        { "PACC0_CPU0", { 0, "mJ" } },
        { "PACC0_CPU1", { 0, "mJ" } },
        { "PACC0_CPU2", { 0, "mJ" } },
        { "PACC0_CPU3", { 0, "mJ" } },
        { "PACC0_CPU4", { 0, "mJ" } },
        { "PACC0_CPU5", { 0, "mJ" } },

        { "PACC1_CPU0", { 1, "mJ" } },
        { "PACC1_CPU1", { 908, "mJ" } },
        { "PACC1_CPU2", { 932, "mJ" } },
        { "PACC1_CPU3", { 275, "mJ" } },
        { "PACC1_CPU4", { 1, "mJ" } },
        { "PACC1_CPU5", { 0, "mJ" } },

        { "EACC_CPM", { 0, "mJ" } },
        { "PACC0_CPM", { 0, "mJ" } },
        { "PACC1_CPM", { 0, "mJ" } },

        { "CPU Energy", { 2118, "mJ" } },
        { "GPU SRAM", { 0, "mJ" } },
        { "ANE", { 0, "mJ" } },
        { "DRAM", { 78, "mJ" } },
        { "GPU Energy", { 80707304, "nJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    AppleEnergyMetrics expected = {
        std::make_optional(2118),
        std::make_optional(std::vector<int64_t> { 1, 1, 0, 0 }),
        std::make_optional(std::vector<int64_t> { 0, 0, 0, 0, 0, 0, 1, 908, 932, 275, 1, 0 }),
        std::make_optional(0),
        std::make_optional(0),
        std::make_optional(78),
        std::make_optional(80),
        std::make_optional(0),
        std::make_optional(0)
    };

    compare_results(result, expected);

    std::cout << "  > test_m4_pro_example passed.\n";
}

int main()
{
    std::cout << "--- Starting tests ---\n";

    test_one_interval();
    test_unit_handling();
    test_m1_max_example();
    test_m3_pro_example();
    test_m4_example();
    test_m4_pro_example();

    std::cout << "--- All tests passed ---\n";
}
