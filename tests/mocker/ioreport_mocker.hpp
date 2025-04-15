#pragma once
#include <string>
#include <unordered_map>
#include <utility>

/* ----- Mock Control Interface -----

This is essentially a very thin RAII-like wrapper to
ensure mock data is cleared & reset between tests.

A sample of mock data can be added to the mocker using
the `push_back_sample` method.

Every time `get_cumulative_energy`, `begin_window`, or
`end_window` is called on an `AppleEnergyMonitor` instance
that was compiled via linking with `ioreport_mocker.cpp`
instead of the real IOReport library, the `AppleEnergyMonitor`
instance will consume a sample provided to the mocker, in the
order that they were inserted via `push_back_sample`.

You can use `set_sample_index` to revert back to a prior
sample that you added via `push_back_sample`. Then, the
`AppleEnergyMonitor` instance will consume that sample and
following samples in the same order they were originally added.

Each sample itself mocks data that an `AppleEnergyMonitor`
instance would obtain from the real IOReport library. A sample
is a map of key-value pairs, where the key is the name of a
field that IOReport might report (e.g., "CPU Energy", "ANE Energy"),
and the value is a pair consisting of an integer value and a unit.

For more detailed information on what fields an `AppleEnergyMonitor`
expects to see in a typical IOReport sample, refer to the
implementation of `AppleEnergyMonitor`, located in `apple_energy.hpp`.
*/

class Mocker {
public:
    Mocker();
    ~Mocker();

    /* Map of: { key: `field_name`, value: (`value`, `unit`) } */
    void push_back_sample(const std::unordered_map<std::string, std::pair<int64_t, std::string>>& data);

    void pop_back_sample();
    void clear_all_mock_samples();
    void set_sample_index(uint64_t index);
};
