#pragma once
#include <string>
#include <unordered_map>

/* ----- Mock Control Interface -----

This is essentially a very thin RAII-like wrapper to
ensure mock data is cleared & reset between tests. */
class Mocker {
public:
    Mocker();
    ~Mocker();
    void push_back_sample(const std::unordered_map<std::string, int64_t>& data);
    void pop_back_sample();
    void clear_all_mock_samples();
    void set_sample_index(uint64_t index);
};
