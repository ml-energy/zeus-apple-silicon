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

        { "EACC_CPU", { 0, "mJ" } },
        { "PACC0_CPU", { 0, "mJ" } },
        { "PACC1_CPU", { 0, "mJ" } },

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

        { "EACC_CPU", { 10, "mJ" } },
        { "PACC0_CPU", { 1700, "mJ" } },
        { "PACC1_CPU", { 5, "mJ" } },

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
        std::make_optional(std::vector<int64_t> { 10 }),
        std::make_optional(std::vector<int64_t> { 1700, 5 }),
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

        { "ECPU", { 0, "mJ" } },
        { "PCPU", { 0, "mJ" } },

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

        { "ECPU", { 15, "mJ" } },
        { "PCPU", { 25, "mJ" } },

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
        std::make_optional(std::vector<int64_t> { 15 }),
        std::make_optional(std::vector<int64_t> { 25 }),
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

        { "ECPU", { 0, "mJ" } },
        { "PCPU", { 0, "mJ" } },

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

        { "ECPU", { 8, "mJ" } },
        { "PCPU", { 2200, "mJ" } },

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
        std::make_optional(std::vector<int64_t> { 8 }),
        std::make_optional(std::vector<int64_t> { 2200 }),
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

        { "EACC_CPU", { 0, "mJ" } },
        { "PACC0_CPU", { 0, "mJ" } },
        { "PACC1_CPU", { 0, "mJ" } },

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

        { "EACC_CPU", { 5, "mJ" } },
        { "PACC0_CPU", { 3, "mJ" } },
        { "PACC1_CPU", { 2120, "mJ" } },

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
        std::make_optional(std::vector<int64_t> { 5 }),
        std::make_optional(std::vector<int64_t> { 3, 2120 }),
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

void test_restart_window()
{
    std::cout << "* Running test_restart_window\n";
    Mocker mocker;

    // Sample for first begin_window
    mocker.push_back_sample({
        { "CPU Energy", { 10000000, "mJ" } },
        { "GPU Energy", { 1000000, "nJ" } },
    });
    // Sample for restarted begin_window
    mocker.push_back_sample({
        { "CPU Energy", { 20000000, "mJ" } },
        { "GPU Energy", { 2000000, "nJ" } },
    });
    // Sample for end_window
    mocker.push_back_sample({
        { "CPU Energy", { 50000000, "mJ" } },
        { "GPU Energy", { 5000000, "nJ" } },
    });

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");

    // Without restart, should throw.
    bool threw = false;
    try {
        monitor.begin_window("test");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);

    // With restart=true, should succeed.
    monitor.begin_window("test", true);

    AppleEnergyMetrics result = monitor.end_window("test");
    // Delta is from the restarted sample (20M) to end sample (50M).
    assert(result.cpu_total_mj.value() == 30000000);
    assert(result.gpu_mj.value() == 3);

    // restart=true on a non-existent window should just start normally.
    mocker.push_back_sample({
        { "CPU Energy", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
    });
    mocker.push_back_sample({
        { "CPU Energy", { 100, "mJ" } },
        { "GPU Energy", { 1000000, "nJ" } },
    });
    monitor.begin_window("new_key", true);
    AppleEnergyMetrics result2 = monitor.end_window("new_key");
    assert(result2.cpu_total_mj.value() == 100);

    std::cout << "  > test_restart_window passed.\n";
}

void test_m5_max_example()
{
    std::cout << "* Running test_m5_max_example\n";
    Mocker mocker;

    // M5 Max: 12 E-cores (2 clusters of 6), 6 P-cores, new naming scheme.
    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        // E-core cluster 0 (individual cores)
        { "MCPU0_0", { 0, "mJ" } },
        { "MCPU0_1", { 0, "mJ" } },
        { "MCPU0_2", { 0, "mJ" } },
        { "MCPU0_3", { 0, "mJ" } },
        { "MCPU0_4", { 0, "mJ" } },
        { "MCPU0_5", { 0, "mJ" } },
        // E-core cluster 1 (individual cores)
        { "MCPU1_0", { 0, "mJ" } },
        { "MCPU1_1", { 0, "mJ" } },
        { "MCPU1_2", { 0, "mJ" } },
        { "MCPU1_3", { 0, "mJ" } },
        { "MCPU1_4", { 0, "mJ" } },
        { "MCPU1_5", { 0, "mJ" } },
        // P-cores
        { "PACC_0", { 0, "mJ" } },
        { "PACC_1", { 0, "mJ" } },
        { "PACC_2", { 0, "mJ" } },
        { "PACC_3", { 0, "mJ" } },
        { "PACC_4", { 0, "mJ" } },
        { "PACC_5", { 0, "mJ" } },
        // E-core managers
        { "MCPM0", { 0, "mJ" } },
        { "MCPM1", { 0, "mJ" } },
        // P-core manager
        { "PCPM", { 0, "mJ" } },
        // E-core cluster totals
        { "MCPU0", { 0, "mJ" } },
        { "MCPU1", { 0, "mJ" } },
        // P-core cluster total
        { "PCPU", { 0, "mJ" } },
        // SRAM channels (should NOT be parsed as cores)
        { "MCPU0_0_SRAM", { 0, "mJ" } },
        { "PCPU0_SRAM", { 0, "mJ" } },
        // DTL channels (should NOT be parsed as cores)
        { "MCPU0DTL00", { 0, "mJ" } },
        { "PCPUDTL00", { 0, "mJ" } },
        // System channels
        { "CPU Energy", { 0, "mJ" } },
        { "DRAM", { 0, "mJ" } },
        { "GPU Energy", { 0, "nJ" } },
        { "ANE", { 0, "mJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        { "MCPU0_0", { 100, "mJ" } },
        { "MCPU0_1", { 90, "mJ" } },
        { "MCPU0_2", { 80, "mJ" } },
        { "MCPU0_3", { 70, "mJ" } },
        { "MCPU0_4", { 60, "mJ" } },
        { "MCPU0_5", { 50, "mJ" } },
        { "MCPU1_0", { 95, "mJ" } },
        { "MCPU1_1", { 85, "mJ" } },
        { "MCPU1_2", { 75, "mJ" } },
        { "MCPU1_3", { 65, "mJ" } },
        { "MCPU1_4", { 55, "mJ" } },
        { "MCPU1_5", { 45, "mJ" } },
        { "PACC_0", { 500, "mJ" } },
        { "PACC_1", { 400, "mJ" } },
        { "PACC_2", { 300, "mJ" } },
        { "PACC_3", { 250, "mJ" } },
        { "PACC_4", { 200, "mJ" } },
        { "PACC_5", { 150, "mJ" } },
        { "MCPM0", { 30, "mJ" } },
        { "MCPM1", { 25, "mJ" } },
        { "PCPM", { 40, "mJ" } },
        { "MCPU0", { 9999, "mJ" } },
        { "MCPU1", { 8888, "mJ" } },
        { "PCPU", { 7777, "mJ" } },
        { "MCPU0_0_SRAM", { 10, "mJ" } },
        { "PCPU0_SRAM", { 20, "mJ" } },
        { "MCPU0DTL00", { 5, "mJ" } },
        { "PCPUDTL00", { 6, "mJ" } },
        { "CPU Energy", { 2000, "mJ" } },
        { "DRAM", { 100, "mJ" } },
        { "GPU Energy", { 5000000, "nJ" } },
        { "ANE", { 10, "mJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    assert(result.cpu_total_mj.value() == 2000);

    // 12 E-cores total (two clusters of 6).
    assert(result.efficiency_cores_mj);
    assert(result.efficiency_cores_mj->size() == 12);
    // Sort to check values regardless of order.
    std::vector<int64_t> ecores = result.efficiency_cores_mj.value();
    std::sort(ecores.begin(), ecores.end());
    std::vector<int64_t> ecores_expected = { 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 };
    assert(ecores == ecores_expected);

    // 6 P-cores.
    assert(result.performance_cores_mj);
    assert(result.performance_cores_mj->size() == 6);
    std::vector<int64_t> pcores = result.performance_cores_mj.value();
    std::sort(pcores.begin(), pcores.end());
    std::vector<int64_t> pcores_expected = { 150, 200, 250, 300, 400, 500 };
    assert(pcores == pcores_expected);

    // E-core cluster totals.
    assert(result.efficiency_cluster_mj);
    std::vector<int64_t> eclusters = result.efficiency_cluster_mj.value();
    std::sort(eclusters.begin(), eclusters.end());
    std::vector<int64_t> eclusters_expected = { 8888, 9999 };
    assert(eclusters == eclusters_expected);

    // P-core cluster total.
    assert(result.performance_cluster_mj);
    std::vector<int64_t> pclusters = result.performance_cluster_mj.value();
    std::vector<int64_t> pclusters_expected = { 7777 };
    assert(pclusters == pclusters_expected);

    // E-core manager: sum of MCPM0 + MCPM1 = 55.
    assert(result.efficiency_core_manager_mj.value() == 55);
    // P-core manager: PCPM = 40.
    assert(result.performance_core_manager_mj.value() == 40);

    assert(result.dram_mj.value() == 100);
    assert(result.gpu_mj.value() == 5);
    assert(result.ane_mj.value() == 10);

    std::cout << "  > test_m5_max_example passed.\n";
}

void test_ultra_example()
{
    std::cout << "* Running test_ultra_example\n";
    Mocker mocker;

    // Ultra chip: two dies, channel names prefixed with DIE_0_ / DIE_1_.
    std::unordered_map<std::string, std::pair<int64_t, std::string>> data1 = {
        // Die 0
        { "DIE_0_EACC_CPU0", { 0, "mJ" } },
        { "DIE_0_EACC_CPU1", { 0, "mJ" } },
        { "DIE_0_EACC_CPU2", { 0, "mJ" } },
        { "DIE_0_EACC_CPU3", { 0, "mJ" } },
        { "DIE_0_PACC0_CPU0", { 0, "mJ" } },
        { "DIE_0_PACC0_CPU1", { 0, "mJ" } },
        { "DIE_0_PACC0_CPU2", { 0, "mJ" } },
        { "DIE_0_PACC0_CPU3", { 0, "mJ" } },
        { "DIE_0_EACC_CPU", { 0, "mJ" } },
        { "DIE_0_PACC0_CPU", { 0, "mJ" } },
        { "DIE_0_EACC_CPM", { 0, "mJ" } },
        { "DIE_0_PACC0_CPM", { 0, "mJ" } },
        { "DIE_0_CPU Energy", { 0, "mJ" } },
        // Die 1
        { "DIE_1_EACC_CPU0", { 0, "mJ" } },
        { "DIE_1_EACC_CPU1", { 0, "mJ" } },
        { "DIE_1_EACC_CPU2", { 0, "mJ" } },
        { "DIE_1_EACC_CPU3", { 0, "mJ" } },
        { "DIE_1_PACC0_CPU0", { 0, "mJ" } },
        { "DIE_1_PACC0_CPU1", { 0, "mJ" } },
        { "DIE_1_PACC0_CPU2", { 0, "mJ" } },
        { "DIE_1_PACC0_CPU3", { 0, "mJ" } },
        { "DIE_1_EACC_CPU", { 0, "mJ" } },
        { "DIE_1_PACC0_CPU", { 0, "mJ" } },
        { "DIE_1_EACC_CPM", { 0, "mJ" } },
        { "DIE_1_PACC0_CPM", { 0, "mJ" } },
        { "DIE_1_CPU Energy", { 0, "mJ" } },
        // Shared (single die for GPU, per-die suffix for block channels)
        { "GPU Energy", { 0, "nJ" } },
        { "DRAM0", { 0, "mJ" } },
        { "DRAM1", { 0, "mJ" } },
        { "ANE0", { 0, "mJ" } },
        { "ANE1", { 0, "mJ" } },
        { "GPU SRAM0", { 0, "mJ" } },
        { "GPU SRAM1", { 0, "mJ" } },
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, std::pair<int64_t, std::string>> data2 = {
        // Die 0
        { "DIE_0_EACC_CPU0", { 10, "mJ" } },
        { "DIE_0_EACC_CPU1", { 8, "mJ" } },
        { "DIE_0_EACC_CPU2", { 6, "mJ" } },
        { "DIE_0_EACC_CPU3", { 4, "mJ" } },
        { "DIE_0_PACC0_CPU0", { 500, "mJ" } },
        { "DIE_0_PACC0_CPU1", { 300, "mJ" } },
        { "DIE_0_PACC0_CPU2", { 200, "mJ" } },
        { "DIE_0_PACC0_CPU3", { 100, "mJ" } },
        { "DIE_0_EACC_CPU", { 30, "mJ" } },
        { "DIE_0_PACC0_CPU", { 1200, "mJ" } },
        { "DIE_0_EACC_CPM", { 5, "mJ" } },
        { "DIE_0_PACC0_CPM", { 15, "mJ" } },
        { "DIE_0_CPU Energy", { 1500, "mJ" } },
        // Die 1
        { "DIE_1_EACC_CPU0", { 12, "mJ" } },
        { "DIE_1_EACC_CPU1", { 9, "mJ" } },
        { "DIE_1_EACC_CPU2", { 7, "mJ" } },
        { "DIE_1_EACC_CPU3", { 3, "mJ" } },
        { "DIE_1_PACC0_CPU0", { 450, "mJ" } },
        { "DIE_1_PACC0_CPU1", { 350, "mJ" } },
        { "DIE_1_PACC0_CPU2", { 250, "mJ" } },
        { "DIE_1_PACC0_CPU3", { 150, "mJ" } },
        { "DIE_1_EACC_CPU", { 35, "mJ" } },
        { "DIE_1_PACC0_CPU", { 1300, "mJ" } },
        { "DIE_1_EACC_CPM", { 6, "mJ" } },
        { "DIE_1_PACC0_CPM", { 18, "mJ" } },
        { "DIE_1_CPU Energy", { 1600, "mJ" } },
        // Shared
        { "GPU Energy", { 50000000, "nJ" } },
        { "DRAM0", { 200, "mJ" } },
        { "DRAM1", { 180, "mJ" } },
        { "ANE0", { 20, "mJ" } },
        { "ANE1", { 15, "mJ" } },
        { "GPU SRAM0", { 40, "mJ" } },
        { "GPU SRAM1", { 30, "mJ" } },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;
    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    // CPU Energy accumulates across dies: 1500 + 1600 = 3100
    assert(result.cpu_total_mj.value() == 3100);

    // 8 E-cores total (4 per die)
    assert(result.efficiency_cores_mj);
    assert(result.efficiency_cores_mj->size() == 8);
    std::vector<int64_t> ecores = result.efficiency_cores_mj.value();
    std::sort(ecores.begin(), ecores.end());
    std::vector<int64_t> ecores_expected = { 3, 4, 6, 7, 8, 9, 10, 12 };
    assert(ecores == ecores_expected);

    // 8 P-cores total (4 per die)
    assert(result.performance_cores_mj);
    assert(result.performance_cores_mj->size() == 8);
    std::vector<int64_t> pcores = result.performance_cores_mj.value();
    std::sort(pcores.begin(), pcores.end());
    std::vector<int64_t> pcores_expected = { 100, 150, 200, 250, 300, 350, 450, 500 };
    assert(pcores == pcores_expected);

    // E-core cluster totals: one per die
    assert(result.efficiency_cluster_mj);
    std::vector<int64_t> eclusters = result.efficiency_cluster_mj.value();
    std::sort(eclusters.begin(), eclusters.end());
    std::vector<int64_t> eclusters_expected = { 30, 35 };
    assert(eclusters == eclusters_expected);

    // P-core cluster totals: one per die
    assert(result.performance_cluster_mj);
    std::vector<int64_t> pclusters = result.performance_cluster_mj.value();
    std::sort(pclusters.begin(), pclusters.end());
    std::vector<int64_t> pclusters_expected = { 1200, 1300 };
    assert(pclusters == pclusters_expected);

    // Managers accumulate across dies
    assert(result.efficiency_core_manager_mj.value() == 11); // 5 + 6
    assert(result.performance_core_manager_mj.value() == 33); // 15 + 18

    // Block channels accumulate across per-die suffixes
    assert(result.dram_mj.value() == 380); // 200 + 180
    assert(result.gpu_mj.value() == 50); // 50M nJ = 50 mJ
    assert(result.gpu_sram_mj.value() == 70); // 40 + 30
    assert(result.ane_mj.value() == 35); // 20 + 15

    std::cout << "  > test_ultra_example passed.\n";
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
    test_restart_window();
    test_m5_max_example();
    test_ultra_example();

    std::cout << "--- All tests passed ---\n";
}
