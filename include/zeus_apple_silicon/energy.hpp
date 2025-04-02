#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <CoreFoundation/CoreFoundation.h>


/* An alias for the subscription reference (forward declared). */
typedef struct __IOReportSubscriptionRef *IOReportSubscriptionRef;

extern "C" {
    /* Returns a dictionary containing channels for the given group. */
    CFDictionaryRef IOReportCopyChannelsInGroup(
        CFStringRef group,
        CFStringRef subgroup,
        uint64_t a,
        uint64_t b,
        uint64_t c
    );

    /* Creates an IOReport subscription based on a mutable channels dictionary. */
    IOReportSubscriptionRef IOReportCreateSubscription(
        const void *unused1,
        CFMutableDictionaryRef channels_dict,
        CFMutableDictionaryRef *unused2,
        uint64_t unused3,
        CFTypeRef unused4
    );

    /* Creates a sample (snapshot) from the subscription. */
    CFDictionaryRef IOReportCreateSamples(
        IOReportSubscriptionRef subs,
        CFMutableDictionaryRef channels_dict_mut,
        const void *unused
    );

    /* Creates a “delta” sample from two snapshots. */
    CFDictionaryRef IOReportCreateSamplesDelta(
        CFDictionaryRef sample1,
        CFDictionaryRef sample2,
        const void *unused
    );

    /* Returns a simple integer value from the channel’s dictionary. */
    int64_t IOReportSimpleGetIntegerValue(
        CFDictionaryRef item,
        int unused
    );

    /* Returns a CFString representing the channel’s name. */
    CFStringRef IOReportChannelGetChannelName(
        CFDictionaryRef item
    );

    /* Returns the units of a channel. */
    CFStringRef IOReportChannelGetUnitLabel(
        CFDictionaryRef item
    );
}

// Represents a single parsed measurement reported by `AppleEnergyMonitor`.
// Units are in mJ.
struct AppleEnergyMetrics {
    int64_t cpu_mj = 0;
    int64_t gpu_mj = 0;
    int64_t ane_mj = 0;
    int64_t dram_mj = 0;

    AppleEnergyMetrics operator-(const AppleEnergyMetrics& other) const {
        AppleEnergyMetrics result;
        result.cpu_mj = cpu_mj - other.cpu_mj;
        result.gpu_mj = gpu_mj - other.gpu_mj;
        result.ane_mj = ane_mj - other.ane_mj;
        result.dram_mj = dram_mj - other.dram_mj;
        return result;
    }
};


class AppleEnergyMonitor {
public:
    AppleEnergyMonitor() {
        initialize();
    }
    ~AppleEnergyMonitor() {
        CFRelease(subscription);
        CFRelease(channels_dict_mutable);

        for (auto it = begin_samples.begin(); it != begin_samples.end(); ++it) {
            CFRelease(it->second);
        }
    }

    AppleEnergyMetrics get_cumulative_energy() {
        AppleEnergyMetrics result;

        CFDictionaryRef sample = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);

        // The sample is expected to contain the channels under the key "IOReportChannels".
        CFStringRef key_str = CFStringCreateWithCString(nullptr, "IOReportChannels", kCFStringEncodingUTF8);
        const void *channels_value = CFDictionaryGetValue(sample, key_str);
        CFRelease(key_str);
        
        if (channels_value == nullptr) {
            CFRelease(sample);
            throw std::runtime_error("No IOReportChannels key found in sample.");
        }

        // Channels_value should be a CFArrayRef of channel dictionaries.
        CFArrayRef channels_array = static_cast<CFArrayRef>(channels_value);
        CFIndex array_count = CFArrayGetCount(channels_array);

        // Iterate through all channels in our subscription.
        for (CFIndex i = 0; i < array_count; i++) {
            const void *item_ptr = CFArrayGetValueAtIndex(channels_array, i);
            CFDictionaryRef item = static_cast<CFDictionaryRef>(item_ptr);

            // Get the channel's name.
            CFStringRef cf_channel_name = IOReportChannelGetChannelName(item);
            std::string channel_name = to_std_string(cf_channel_name);

            // Get the channel's unit label.
            CFStringRef cf_unit = IOReportChannelGetUnitLabel(item);
            std::string unit = to_std_string(cf_unit);

            if (channel_name.find("GPU Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
                result.gpu_mj = energy / 1e6;
            }
            else if (channel_name.find("CPU Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
                result.cpu_mj = energy;
            }
            else if (channel_name.find("ANE Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
            }
            else if (channel_name.find("DRAM Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
            }

        }

        CFRelease(sample);
        return result;
    }

    void begin_window(const std::string& key) {
        if (begin_samples.find(key) != begin_samples.end()) {
            throw std::runtime_error("Measurement with specified key already exists.");
        }
        begin_samples[key] = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);
    }

    AppleEnergyMetrics end_window(const std::string& key) {
        auto it = begin_samples.find(key);
        if (it == begin_samples.end()) {
            throw std::runtime_error("No measurement with provided key had been started."); 
        }

        CFDictionaryRef sample1 = it->second;
        CFDictionaryRef sample2 = IOReportCreateSamples(subscription, channels_dict_mutable, nullptr);

        // Create a delta sample (the differences between the two snapshots).
        CFDictionaryRef sample_delta = IOReportCreateSamplesDelta(sample1, sample2, nullptr);
        CFRelease(sample1);
        CFRelease(sample2);
        begin_samples.erase(key);

        // The delta sample is expected to contain the channels under the key "IOReportChannels".
        CFStringRef key_str = CFStringCreateWithCString(nullptr, "IOReportChannels", kCFStringEncodingUTF8);
        const void *channels_value = CFDictionaryGetValue(sample_delta, key_str);
        CFRelease(key_str);

        if (channels_value == nullptr) {
            CFRelease(sample_delta);
            throw std::runtime_error("No IOReportChannels key found in sample delta.");
        }

        AppleEnergyMetrics result;
        
        // Channels_value should be a CFArrayRef of channel dictionaries.
        CFArrayRef channels_array = static_cast<CFArrayRef>(channels_value);
        CFIndex array_count = CFArrayGetCount(channels_array);
        
        // Iterate through all channels in our subscription.
        for (CFIndex i = 0; i < array_count; i++) {
            const void *item_ptr = CFArrayGetValueAtIndex(channels_array, i);
            CFDictionaryRef item = static_cast<CFDictionaryRef>(item_ptr);
            
            // Get the channel's name.
            CFStringRef cf_channel_name = IOReportChannelGetChannelName(item);
            std::string channel_name = to_std_string(cf_channel_name);
            
            // Get the channel's unit label.
            CFStringRef cf_unit = IOReportChannelGetUnitLabel(item);
            std::string unit = to_std_string(cf_unit);
            
            if (channel_name.find("GPU Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
                result.gpu_mj = energy / 1e6;
            }
            else if (channel_name.find("CPU Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
                result.cpu_mj = energy;
            }
            else if (channel_name.find("ANE Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
            }
            else if (channel_name.find("DRAM Energy") != std::string::npos) {
                int64_t energy = IOReportSimpleGetIntegerValue(item, 0);
            }

        }

        CFRelease(sample_delta);
        return result;
    }

private:
    std::unordered_map<std::string, CFDictionaryRef> begin_samples;

    IOReportSubscriptionRef subscription;
    CFMutableDictionaryRef channels_dict_mutable;

    void initialize() {
        const char* group_cstr = "Energy Model";
        CFStringRef group_name = CFStringCreateWithCString(nullptr, group_cstr, kCFStringEncodingUTF8);
        CFDictionaryRef channels_dict = IOReportCopyChannelsInGroup(group_name, nullptr, 0, 0, 0);

        // Release string to avoid memory leak.
        CFRelease(group_name);

        // Make a mutable copy of the channels dictionary (needed for subscriptions).
        CFIndex num_items = CFDictionaryGetCount(channels_dict);

        channels_dict_mutable = CFDictionaryCreateMutableCopy(nullptr, num_items, channels_dict);
        CFRelease(channels_dict);

        // Create the IOReport subscription.
        CFMutableDictionaryRef updatedChannels = nullptr;

        subscription = IOReportCreateSubscription(
            nullptr, channels_dict_mutable, &updatedChannels, 0, nullptr
        );

        if (subscription == nullptr) {
            std::cerr << "Failed to create subscription" << '\n';
            return;
        }
    }

    std::string to_std_string(CFStringRef cf_str) {
        char buffer[256];
        bool ok = CFStringGetCString(cf_str, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        if (ok) {
            return std::string(buffer);
        }
        throw std::runtime_error("Failed to convert CFString to std::string");
    }
};
