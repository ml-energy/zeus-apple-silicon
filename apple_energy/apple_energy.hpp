#pragma once

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <CoreFoundation/CoreFoundation.h>

/* An alias for the subscription reference (forward declared). */
typedef struct __IOReportSubscriptionRef* IOReportSubscriptionRef;

extern "C" {
/* Returns a dictionary containing channels for the given group. */
CFDictionaryRef IOReportCopyChannelsInGroup(CFStringRef group,
    CFStringRef subgroup, uint64_t a,
    uint64_t b, uint64_t c);

/* Creates an IOReport subscription based on a mutable channels dictionary. */
IOReportSubscriptionRef IOReportCreateSubscription(
    const void* unused1, CFMutableDictionaryRef channels_dict,
    CFMutableDictionaryRef* unused2, uint64_t unused3, CFTypeRef unused4);

/* Creates a sample (snapshot) from the subscription. */
CFDictionaryRef IOReportCreateSamples(IOReportSubscriptionRef subs,
    CFMutableDictionaryRef channels_dict_mut,
    const void* unused);

/* Creates a “delta” sample from two snapshots. */
CFDictionaryRef IOReportCreateSamplesDelta(CFDictionaryRef sample1,
    CFDictionaryRef sample2,
    const void* unused);

/* Returns a simple integer value from the channel’s dictionary. */
int64_t IOReportSimpleGetIntegerValue(CFDictionaryRef item, int unused);

/* Returns a CFString representing the channel’s name. */
CFStringRef IOReportChannelGetChannelName(CFDictionaryRef item);

/* Returns the units of a channel. */
CFStringRef IOReportChannelGetUnitLabel(CFDictionaryRef item);
}

/*
Represents a single parsed measurement reported by `AppleEnergyMonitor`.

If a field could not be observed/detected on the current device,
it will be left as an empty `std::optional` object.

Units are in mJ.
*/
struct AppleEnergyMetrics {
    // CPU related metrics
    std::optional<int64_t> cpu_total_mj;
    std::optional<std::vector<int64_t>> efficiency_cores_mj;
    std::optional<std::vector<int64_t>> performance_cores_mj;
    std::optional<std::vector<int64_t>> efficiency_cluster_mj;
    std::optional<std::vector<int64_t>> performance_cluster_mj;
    std::optional<int64_t> efficiency_core_manager_mj;
    std::optional<int64_t> performance_core_manager_mj;

    // DRAM
    std::optional<int64_t> dram_mj;

    // GPU related metrics
    std::optional<int64_t> gpu_mj;
    std::optional<int64_t> gpu_sram_mj;

    // ANE
    std::optional<int64_t> ane_mj;
};

class AppleEnergyMonitor {
public:
    AppleEnergyMonitor() { initialize(); }

    ~AppleEnergyMonitor()
    {
        CFRelease(subscription);
        CFRelease(channels_dict_mutable);

        for (auto it = begin_samples.begin(); it != begin_samples.end(); ++it) {
            CFRelease(it->second);
        }
    }

    void begin_window(const std::string& key, bool restart = false)
    {
        auto it = begin_samples.find(key);
        if (it != begin_samples.end()) {
            if (!restart) {
                throw std::runtime_error(
                    "Measurement with specified key already exists.");
            }
            CFRelease(it->second);
            begin_samples.erase(it);
        }
        begin_samples[key] = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);
    }

    AppleEnergyMetrics end_window(const std::string& key)
    {
        auto it = begin_samples.find(key);
        if (it == begin_samples.end()) {
            throw std::runtime_error(
                "No measurement with provided key had been started.");
        }

        CFDictionaryRef sample1 = it->second;
        CFDictionaryRef sample2 = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);

        // Create a delta sample (the difference between the two cumulative
        // snapshots).
        CFDictionaryRef sample_delta = IOReportCreateSamplesDelta(sample1, sample2, nullptr);
        CFRelease(sample1);
        CFRelease(sample2);
        begin_samples.erase(key);

        // A delta sample is treated the same as a cumulative sample.
        AppleEnergyMetrics result = parse_sample(sample_delta);
        CFRelease(sample_delta);

        return result;
    }

    AppleEnergyMetrics get_cumulative_energy()
    {
        CFDictionaryRef sample = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);
        AppleEnergyMetrics result = parse_sample(sample);
        CFRelease(sample);
        return result;
    }

private:
    std::unordered_map<std::string, CFDictionaryRef> begin_samples;

    IOReportSubscriptionRef subscription;
    CFMutableDictionaryRef channels_dict_mutable;

    void initialize()
    {
        const char* group_cstr = "Energy Model";
        CFStringRef group_name = CFStringCreateWithCString(nullptr, group_cstr, kCFStringEncodingUTF8);
        CFDictionaryRef channels_dict = IOReportCopyChannelsInGroup(group_name, nullptr, 0, 0, 0);

        // Release string to avoid memory leak.
        CFRelease(group_name);

        // Make a mutable copy of the channels dictionary (needed for
        // subscriptions).
        CFIndex num_items = CFDictionaryGetCount(channels_dict);

        channels_dict_mutable = CFDictionaryCreateMutableCopy(nullptr, num_items, channels_dict);
        CFRelease(channels_dict);

        // Create the IOReport subscription.
        CFMutableDictionaryRef updatedChannels = nullptr;
        subscription = IOReportCreateSubscription(nullptr, channels_dict_mutable,
            &updatedChannels, 0, nullptr);

        if (subscription == nullptr) {
            throw std::runtime_error("Failed to create IOReport subscription.");
        }
    }

    AppleEnergyMetrics parse_sample(CFDictionaryRef sample)
    {
        AppleEnergyMetrics result;

        // The sample is expected to contain the channels under the key
        // "IOReportChannels".
        CFStringRef key_str = CFStringCreateWithCString(nullptr, "IOReportChannels",
            kCFStringEncodingUTF8);
        const void* channels_value = CFDictionaryGetValue(sample, key_str);
        CFRelease(key_str);

        // If parsing the sample was not possible, return a result having all fields
        // set to an empty `std::optional`, indicating that all fields are
        // unobservable.
        if (channels_value == nullptr) {
            return result;
        }

        // `channels_value` should be a CFArrayRef of channel dictionaries.
        CFArrayRef channels_array = static_cast<CFArrayRef>(channels_value);
        CFIndex array_count = CFArrayGetCount(channels_array);

        // Iterate through all channels in our subscription.
        for (CFIndex i = 0; i < array_count; i++) {
            const void* item_ptr = CFArrayGetValueAtIndex(channels_array, i);
            CFDictionaryRef item = static_cast<CFDictionaryRef>(item_ptr);

            // Get the channel's name and unit label.
            CFStringRef cf_channel_name = IOReportChannelGetChannelName(item);
            CFStringRef cf_unit = IOReportChannelGetUnitLabel(item);

            std::string channel_name = to_std_string(cf_channel_name);
            int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
            std::string unit = to_std_string(cf_unit);

            energy = convert_to_mj(energy, unit);

            // Multi-die (Ultra) chips prefix channel names with "DIE_N_".
            // Strip this prefix so downstream matchers work on the base name.
            std::string base_name = strip_die_prefix(channel_name);

            if (base_name.find("CPU Energy") != std::string::npos) {
                result.cpu_total_mj = result.cpu_total_mj.value_or(0) + energy;
            } else if (is_efficiency_core(base_name)) {
                if (!result.efficiency_cores_mj) {
                    result.efficiency_cores_mj = std::vector<int64_t>();
                }
                result.efficiency_cores_mj->push_back(energy);
            } else if (is_performance_core(base_name)) {
                if (!result.performance_cores_mj) {
                    result.performance_cores_mj = std::vector<int64_t>();
                }
                result.performance_cores_mj->push_back(energy);
            } else if (is_efficiency_cluster(base_name)) {
                if (!result.efficiency_cluster_mj) {
                    result.efficiency_cluster_mj = std::vector<int64_t>();
                }
                result.efficiency_cluster_mj->push_back(energy);
            } else if (is_performance_cluster(base_name)) {
                if (!result.performance_cluster_mj) {
                    result.performance_cluster_mj = std::vector<int64_t>();
                }
                result.performance_cluster_mj->push_back(energy);
            } else if (is_efficiency_manager(base_name)) {
                result.efficiency_core_manager_mj = result.efficiency_core_manager_mj.value_or(0) + energy;
            } else if (is_performance_manager(base_name)) {
                result.performance_core_manager_mj = result.performance_core_manager_mj.value_or(0) + energy;
            } else if (base_name.find("DRAM") != std::string::npos) {
                result.dram_mj = result.dram_mj.value_or(0) + energy;
            } else if (base_name.find("GPU Energy") != std::string::npos) {
                result.gpu_mj = result.gpu_mj.value_or(0) + energy;
            } else if (base_name.find("GPU SRAM") != std::string::npos) {
                result.gpu_sram_mj = result.gpu_sram_mj.value_or(0) + energy;
            } else if (base_name.find("ANE") != std::string::npos) {
                result.ane_mj = result.ane_mj.value_or(0) + energy;
            }
        }

        return result;
    }

    std::string to_std_string(CFStringRef cf_str)
    {
        char buffer[256];
        bool ok = CFStringGetCString(cf_str, buffer, sizeof(buffer),
            kCFStringEncodingUTF8);
        if (ok) {
            return std::string(buffer);
        }
        throw std::runtime_error("Failed to convert CFString to std::string");
    }

    // Multi-die (Ultra) chips prefix IOReport channel names with "DIE_N_".
    // Strip this so all downstream matchers work on the base name.
    std::string strip_die_prefix(const std::string& name)
    {
        if (name.size() > 4 && name.substr(0, 4) == "DIE_") {
            std::size_t second_underscore = name.find('_', 4);
            if (second_underscore != std::string::npos && second_underscore + 1 < name.size()) {
                return name.substr(second_underscore + 1);
            }
        }
        return name;
    }

    // Checks if a channel name represents an individual CPU core energy reading.
    //
    // Efficiency core patterns:
    //   M1-M4:  ECPU0, ECPU1, EACC_CPU0, EACC_CPU1, ...
    //   M5:     MCPU0_0, MCPU0_1, MCPU1_0, ...
    //
    // Performance core patterns:
    //   M1-M4:  PCPU0, PCPU1, PACC0_CPU0, PACC1_CPU0, ...
    //   M5:     PACC_0, PACC_1, ...
    bool is_efficiency_core(const std::string& s)
    {
        // M1-M4: starts with 'E', contains "CPU" followed by digits.
        // e.g., ECPU0, EACC_CPU0
        if (!s.empty() && s[0] == 'E') {
            return matches_cpu_digit_suffix(s);
        }
        // M5: "MCPUx_y" where x and y are digits.
        // Must not match cluster totals like "MCPU0" (no underscore)
        // or SRAM/DTL channels.
        if (s.size() >= 7 && s.substr(0, 4) == "MCPU"
            && s.find("SRAM") == std::string::npos
            && s.find("DTL") == std::string::npos) {
            std::size_t underscore = s.rfind('_');
            if (underscore != std::string::npos && underscore >= 4) {
                return all_digits(s, underscore + 1);
            }
        }
        return false;
    }

    bool is_performance_core(const std::string& s)
    {
        // M1-M4: starts with 'P', contains "CPU", ends with digits.
        if (!s.empty() && s[0] == 'P' && s.find("CPU") != std::string::npos) {
            return matches_cpu_digit_suffix(s);
        }
        // M5: "PACC_y" where y is a digit. Must end with digits after underscore.
        if (s.size() >= 6 && s.substr(0, 5) == "PACC_") {
            return all_digits(s, 5);
        }
        return false;
    }

    bool is_efficiency_manager(const std::string& s)
    {
        // M1-M4: starts with 'E', ends with "CPM" (e.g., ECPM, EACC_CPM).
        if (!s.empty() && s[0] == 'E' && ends_with(s, "CPM")) {
            return true;
        }
        // M5: "MCPMx" where x is a digit (e.g., MCPM0, MCPM1).
        if (s.size() >= 5 && s.substr(0, 4) == "MCPM") {
            return all_digits(s, 4);
        }
        return false;
    }

    bool is_performance_manager(const std::string& s)
    {
        // All generations: starts with 'P', ends with "CPM"
        // (e.g., PCPM, PACC0_CPM, PACC1_CPM).
        return !s.empty() && s[0] == 'P' && ends_with(s, "CPM");
    }

    // Checks if a channel name represents a CPU cluster total (including shared
    // resources like L2 cache), as opposed to an individual core.
    //
    // Efficiency cluster patterns:
    //   M1-M4:  ECPU, EACC_CPU (ends exactly with "CPU", no trailing digit)
    //   M5:     MCPU0, MCPU1 (no underscore, no SRAM/DTL suffix)
    //
    // Performance cluster patterns:
    //   M1-M4:  PCPU, PACC0_CPU, PACC1_CPU (ends exactly with "CPU")
    //   M5:     PCPU (same)
    bool is_efficiency_cluster(const std::string& s)
    {
        // M1-M4: starts with 'E', ends exactly with "CPU".
        if (!s.empty() && s[0] == 'E' && ends_with(s, "CPU")) {
            return true;
        }
        // M5: "MCPUx" — starts with "MCPU", followed by digit(s) only.
        if (s.size() >= 5 && s.substr(0, 4) == "MCPU"
            && s.find('_') == std::string::npos
            && s.find("SRAM") == std::string::npos
            && s.find("DTL") == std::string::npos) {
            return all_digits(s, 4);
        }
        return false;
    }

    bool is_performance_cluster(const std::string& s)
    {
        // All generations: starts with 'P', ends exactly with "CPU".
        return !s.empty() && s[0] == 'P' && ends_with(s, "CPU");
    }

    // Helper: checks if s contains "CPU" and everything after the last "CPU"
    // consists of digits.
    bool matches_cpu_digit_suffix(const std::string& s)
    {
        std::size_t pos = s.rfind("CPU");
        if (pos == std::string::npos) {
            return false;
        }
        std::size_t start = pos + 3;
        return start < s.size() && all_digits(s, start);
    }

    bool ends_with(const std::string& s, const std::string& suffix)
    {
        return s.size() >= suffix.size()
            && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool all_digits(const std::string& s, std::size_t from)
    {
        if (from >= s.size()) {
            return false;
        }
        for (std::size_t i = from; i < s.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
                return false;
            }
        }
        return true;
    }

    int64_t convert_to_mj(int64_t energy, const std::string& unit)
    {
        if (unit == "nJ") {
            return energy / 1'000'000LL;
        } else if (unit == "uJ" || unit == "µJ") {
            return energy / 1'000LL;
        } else if (unit == "mJ") {
            return energy;
        } else if (unit == "cJ") {
            return energy * 10LL;
        } else if (unit == "dJ") {
            return energy * 100LL;
        } else if (unit == "J") {
            return energy * 1'000LL;
        } else if (unit == "daJ") {
            return energy * 10'000LL;
        } else if (unit == "hJ") {
            return energy * 100'000LL;
        } else if (unit == "kJ") {
            return energy * 1'000'000LL;
        } else if (unit == "MJ") {
            return energy * 1'000'000'000LL;
        } else if (unit == "GJ") {
            return energy * 1'000'000'000'000LL;
        } else if (unit == "TJ") {
            return energy * 1'000'000'000'000'000LL;
        } else if (unit == "PJ") {
            return energy * 1'000'000'000'000'000'000LL;
        } else {
            throw std::invalid_argument("Unsupported or invalid energy unit provided: " + unit);
        }
    }
};
