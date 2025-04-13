#include "energy.hpp"
#include "mock_ioreport.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/* Implemented by mock infrastructure. */
void set_mock_sample(const std::unordered_map<std::string, int64_t>& data);
void reset_mock_state();

void test_one_interval()
{
    std::cout << "* Running test_one_interval\n";
    Mocker mocker;

    std::unordered_map<std::string, int64_t> data1 = {
        { "CPU Energy", 10000000 }, // in mJ
        { "GPU Energy", 1000000 }, // in nJ
    };
    mocker.push_back_sample(data1);

    std::unordered_map<std::string, int64_t> data2 = {
        { "CPU Energy", 55000000 },
        { "GPU Energy", 3000000 },
    };
    mocker.push_back_sample(data2);

    AppleEnergyMonitor monitor;

    monitor.begin_window("test");
    AppleEnergyMetrics result = monitor.end_window("test");

    assert(result.cpu_total_mj.value() == 45000000);
    assert(result.gpu_mj.value() == 2);
}

int main()
{
    std::cout << "--- Starting tests ---\n";

    test_one_interval();

    std::cout << "All tests passed.\n";
}
