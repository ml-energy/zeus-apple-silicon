#pragma once
#include <string>
#include <unordered_map>
#include <utility>

/* ----- Mock Control Interface -----

This is essentially a very thin RAII-like wrapper to
ensure mock data is cleared & reset between tests. */
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
