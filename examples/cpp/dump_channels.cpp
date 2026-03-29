// Dump all raw IOReport energy channels on the current Apple Silicon chip.
//
// Use this tool when adding support for a new chip generation to inspect
// what channel names, values, and units the hardware exports. The output
// helps identify naming changes that require updates to the parsing logic
// in apple_energy.hpp.

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <CoreFoundation/CoreFoundation.h>

typedef struct __IOReportSubscriptionRef* IOReportSubscriptionRef;

extern "C" {
CFDictionaryRef IOReportCopyChannelsInGroup(CFStringRef group,
    CFStringRef subgroup, uint64_t a,
    uint64_t b, uint64_t c);

IOReportSubscriptionRef IOReportCreateSubscription(
    const void* unused1, CFMutableDictionaryRef channels_dict,
    CFMutableDictionaryRef* unused2, uint64_t unused3, CFTypeRef unused4);

CFDictionaryRef IOReportCreateSamples(IOReportSubscriptionRef subs,
    CFMutableDictionaryRef channels_dict_mut,
    const void* unused);

int64_t IOReportSimpleGetIntegerValue(CFDictionaryRef item, int unused);
CFStringRef IOReportChannelGetChannelName(CFDictionaryRef item);
CFStringRef IOReportChannelGetUnitLabel(CFDictionaryRef item);
}

std::string to_std_string(CFStringRef cf_str)
{
    char buffer[256];
    if (CFStringGetCString(cf_str, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
        return std::string(buffer);
    }
    return "(unknown)";
}

struct RawChannel {
    std::string name;
    int64_t value;
    std::string unit;
};

std::vector<RawChannel> get_raw_channels(
    IOReportSubscriptionRef subscription,
    CFMutableDictionaryRef channels_dict_mutable)
{
    std::vector<RawChannel> result;

    CFDictionaryRef sample = IOReportCreateSamples(
        subscription, channels_dict_mutable, nullptr);

    CFStringRef key_str = CFStringCreateWithCString(
        nullptr, "IOReportChannels", kCFStringEncodingUTF8);
    const void* channels_value = CFDictionaryGetValue(sample, key_str);
    CFRelease(key_str);

    if (channels_value == nullptr) {
        CFRelease(sample);
        return result;
    }

    CFArrayRef channels_array = static_cast<CFArrayRef>(channels_value);
    CFIndex count = CFArrayGetCount(channels_array);

    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef item = static_cast<CFDictionaryRef>(
            CFArrayGetValueAtIndex(channels_array, i));

        result.push_back({
            to_std_string(IOReportChannelGetChannelName(item)),
            IOReportSimpleGetIntegerValue(item, 0),
            to_std_string(IOReportChannelGetUnitLabel(item)),
        });
    }

    CFRelease(sample);
    return result;
}

int main()
{
    CFStringRef group_name = CFStringCreateWithCString(
        nullptr, "Energy Model", kCFStringEncodingUTF8);
    CFDictionaryRef channels_dict = IOReportCopyChannelsInGroup(
        group_name, nullptr, 0, 0, 0);
    CFRelease(group_name);

    CFIndex num_items = CFDictionaryGetCount(channels_dict);
    CFMutableDictionaryRef channels_dict_mutable = CFDictionaryCreateMutableCopy(
        nullptr, num_items, channels_dict);
    CFRelease(channels_dict);

    CFMutableDictionaryRef updated_channels = nullptr;
    IOReportSubscriptionRef subscription = IOReportCreateSubscription(
        nullptr, channels_dict_mutable, &updated_channels, 0, nullptr);

    if (subscription == nullptr) {
        std::cerr << "Failed to create IOReport subscription.\n";
        CFRelease(channels_dict_mutable);
        return 1;
    }

    auto channels = get_raw_channels(subscription, channels_dict_mutable);

    // Find the longest channel name for alignment.
    size_t max_name_len = 0;
    for (const auto& ch : channels) {
        if (ch.name.size() > max_name_len) {
            max_name_len = ch.name.size();
        }
    }

    std::cout << "IOReport 'Energy Model' channels (" << channels.size() << " total):\n\n";
    for (const auto& ch : channels) {
        std::cout << "  " << std::left << std::setw(max_name_len + 2) << ch.name
                  << std::right << std::setw(20) << ch.value
                  << " " << ch.unit << "\n";
    }

    CFRelease(subscription);
    CFRelease(channels_dict_mutable);
}
