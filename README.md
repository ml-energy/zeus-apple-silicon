# In-Code Energy Measurement on Apple Silicon  ⚡

A lightweight, header-only C++ library for precisely measuring energy consumed by arbitrary code snippets on Apple Silicon processors.

Also available in Python, installable via `pip`.

This library is used to provide macOS support for [Zeus](https://github.com/ml-energy/zeus), a framework for deep learning energy measurement and optimization.


## Installation

### Python

Install the package using pip:

```bash
pip install zeus-apple-silicon
```

Then, import the necessary components in your Python script:

```python
from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics
# or: import zeus_apple_silicon
```

### C++

1.  Copy/move the header file `apple_energy.hpp` (found in the `apple_energy/` directory of the source repository) into your project.
2.  Include the header in your C++ files:
    ```C++
    #include "apple_energy.hpp"
    ```
3.  Link your executable against Apple's `CoreFoundation` framework and the `IOReport` library (part of IOKit); both dependencies are available by default on nearly all macOS devices. The library requires C++17.
    *   Manual Compilation (g++/clang++) - add necessary flags to your compile command:
        ```bash
        g++ your_code.cpp -o your_executable -std=c++17 -framework CoreFoundation -lIOReport
        # or
        clang++ your_code.cpp -o your_executable -std=c++17 -framework CoreFoundation -lIOReport
        ```
    *   CMake - add the following to your `CMakeLists.txt` file, where `your_target_name` is the name of your executable or library target:
        ```cmake
        target_link_libraries(your_target_name PRIVATE
            "-framework CoreFoundation"
            IOReport
        )
        ```

## Usage Overview

The library operates by defining measurement windows. You mark the beginning and end of a code section you want to measure.

1.  **Start a measurement window**: use `AppleEnergyMonitor::begin_window(label)` to indicate you want energy measurement to *start* at that line of code. Each window needs a string label passed in as an argument.
2.  **The code being measured**: The start of the window should be followed by the code you want to measure energy for.
3.  **End & Retrieve Results**: indicate where you want your measurement window to *end* by using `AppleEnergyMonitor::end_window(label)` with the *same label* you used to mark the window's start. The `AppleEnergyMonitor::end_window(label)` function returns an `AppleEnergyMetrics` object containing energy data collected during the window.

**Note about Measurement Windows:**

*   You can have multiple windows active simultaneously (i.e., they can overlap), as long as each uses a distinct label.
*   Non-overlapping windows can re-use names. I.e., once a window is ended with `end_window`, its label is free to be reused.
*   Attempting to start a window with a label still currently in use (i.e., `end_window` not yet called for that label) will raise an exception, unless `restart=True` is passed.
*   Calling `end_window` with a label that doesn't belong to any currently active window will raise an exception.

**Note about Results of Measurements:**
*   Results are reported via an `AppleEnergyMetrics` struct, but depending on your processor, some metrics may not be available (e.g., DRAM may not be available on older machines). In such cases, fields that could not be measured will be presented as: `None` in Python, and an empty `std::optional` object in C++.
*   A more detailed explanation of results is provided [later in this readme](#structclass-appleenergymetrics).

## Usage Examples

The API is identical in C++ and Python.
For available fields of a result object, read [this section of the readme](#structclass-appleenergymetrics).

### C++ Example

```C++
#include "apple_energy.hpp"

int main() {
    // Create a monitor instance.
    AppleEnergyMonitor monitor;

    // --- Basic Measurement ---
    monitor.begin_window("task_1"); // Indicating the measurement window starts here.

    // Do some work...

    // End the window and get results.
    AppleEnergyMetrics result1 = monitor.end_window("task_1");


    // --- Overlapping Measurements ---
    monitor.begin_window("outer_task");

    monitor.begin_window("inner_task");
    AppleEnergyMetrics inner_result = monitor.end_window("inner_task");

    AppleEnergyMetrics outer_result = monitor.end_window("outer_task");


    // --- Reusing a Label ---
    monitor.begin_window("task_1"); // This is okay because previous "task_1" window ended.
    AppleEnergyMetrics result = monitor.end_window("task_1");
}
```

### Python Example

```python
from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics

# Create a monitor instance.
monitor = AppleEnergyMonitor()

# --- Basic Measurement ---
monitor.begin_window("task_1") # Indicating the measurement window starts here.

# Do some work...

# End the window and get results.
result1 = monitor.end_window("task_1")


# --- Overlapping Measurements ---
monitor.begin_window("outer_task")

monitor.begin_window("inner_task")
inner_result = monitor.end_window("inner_task")

outer_result = monitor.end_window("outer_task")


# --- Reusing a Label ---
monitor.begin_window("task_1") # This is okay because previous "task_1" ended.
result = monitor.end_window("task_1")
```


## API Reference

### Class: `AppleEnergyMonitor`

The main class for getting energy measurements.

*   `AppleEnergyMonitor()`: Constructor. Initializes the monitoring system.
*   `begin_window(label: str, restart: bool = False)`: Starts a new measurement window identified by `label`. If `restart` is `True` and a window with the same label is already active, the existing window is cancelled and a new one begins. This is useful in interactive environments like Jupyter notebooks where a cell may crash between `begin_window` and `end_window`.
*   `end_window(label: str) -> AppleEnergyMetrics`: Ends the measurement window identified by `label` and returns an object containing the results.
*   `get_cumulative_energy() -> AppleEnergyMetrics`: Returns cumulative energy consumed from an unspecified point fixed over the lifetime of the energy monitor (e.g., from bootup).

### Struct/Class: `AppleEnergyMetrics`

This struct/class is how results get reported, containing metrics for various SoC subsystems. All energy values are reported in **mJ** (millijoules).

On some hardware configurations, certain metrics may not be available. In such cases:
*   C++: The field will be an empty `std::optional`. Check `.has_value()` before accessing `.value()`.
*   Python: The field will be `None`. Check for `None` before using the value.

**Fields:**

| Field | C++ Type | Python Type | Description |
|---|---|---|---|
| `cpu_total_mj` | `std::optional<int64_t>` | `Optional[int]` | Total energy consumed by all CPU subsystems combined. |
| `efficiency_cores_mj` | `std::optional<std::vector<int64_t>>` | `Optional[list[int]]` | Per-core energy for each efficiency core. |
| `performance_cores_mj` | `std::optional<std::vector<int64_t>>` | `Optional[list[int]]` | Per-core energy for each performance core. |
| `efficiency_cluster_mj` | `std::optional<std::vector<int64_t>>` | `Optional[list[int]]` | Per-cluster energy totals for efficiency core clusters. Includes shared resources (L2 cache, interconnect). |
| `performance_cluster_mj` | `std::optional<std::vector<int64_t>>` | `Optional[list[int]]` | Per-cluster energy totals for performance core clusters. Includes shared resources like L2 cache. |
| `efficiency_core_manager_mj` | `std::optional<int64_t>` | `Optional[int]` | Energy for efficiency core cluster manager logic. |
| `performance_core_manager_mj` | `std::optional<int64_t>` | `Optional[int]` | Energy for performance core cluster manager logic. |
| `dram_mj` | `std::optional<int64_t>` | `Optional[int]` | External DRAM energy. |
| `gpu_mj` | `std::optional<int64_t>` | `Optional[int]` | On-chip GPU energy. |
| `gpu_sram_mj` | `std::optional<int64_t>` | `Optional[int]` | GPU SRAM/cache energy. |
| `ane_mj` | `std::optional<int64_t>` | `Optional[int]` | Apple Neural Engine energy. |

**Cluster totals vs. per-core sums:** Cluster totals (`efficiency_cluster_mj`, `performance_cluster_mj`) are more representative of the actual energy consumed by a group of cores than the sum of individual per-core values. Clusters include shared resources (L2 cache, interconnect) not attributed to individual cores. On the M5 Max, cluster totals can be 1.5x to 2x the per-core sum.

**M5 core naming:** The M5 Pro/Max replace Efficiency cores with a new "Performance" core type alongside "Super" cores (see [Apple M5 on Wikipedia](https://en.wikipedia.org/wiki/Apple_M5)). The library maps Performance cores (`MCPU`) to `efficiency_*` fields and Super cores (`PCPU`) to `performance_*` fields, so `efficiency_*` always holds the lower-power tier. Only the M5 Max has been tested; the base M5 and M5 Pro are expected to work but are unverified.

## How It Works

The library uses Apple's **IOReport** framework, a private (undocumented) macOS framework that provides hardware telemetry. It subscribes to the "Energy Model" channel group, which contains cumulative energy counters for various SoC subsystems. No sudo is required.

### Accuracy

IOReport energy values are believed to be model-based estimates derived from utilization, frequency, and voltage, rather than direct power sensor readings. Resolution is 1 mJ for most channels. Very short measurement windows (under ~10 ms) may be dominated by quantization noise. Apple's `powermetrics` man page notes that reported power values are "estimated and may be inaccurate."


## Adding Support for a New Apple Silicon Chip

When a new Apple Silicon generation is released, the IOReport channel naming may change (as happened with the M5 generation). Follow these steps to verify compatibility and add support:

### 1. Build and inspect raw channels

```bash
cmake -S . -B build --fresh
cmake --build build
./build/bin/dump_channels
```

The `dump_channels` tool prints every IOReport energy channel name, raw value, and unit on the current hardware. Compare the output against the channel patterns recognized in `apple_energy/apple_energy.hpp` (`is_efficiency_core`, `is_performance_core`, `is_efficiency_manager`, `is_performance_manager`, and the substring matches for `CPU Energy`, `DRAM`, `GPU Energy`, `GPU SRAM`, `ANE`).

### 2. Run the Python compatibility checker

```bash
pip install -e .
python examples/python/check_compatibility.py
```

This shows which `AppleEnergyMetrics` fields are available on the current chip. If fields that should be available show as "not available", the parsing logic needs updating.

### 3. Update parsing logic (if needed)

Edit the channel classification functions in `apple_energy/apple_energy.hpp`. For reference, here are the known naming patterns:

| Metric | M1-M4 patterns | M5 patterns |
|---|---|---|
| E-core (individual) | `ECPU0`, `EACC_CPU0` | `MCPU0_0`, `MCPU1_0` |
| P-core (individual) | `PCPU0`, `PACC0_CPU0` | `PACC_0` |
| E-core cluster total | `ECPU`, `EACC_CPU` | `MCPU0`, `MCPU1` |
| P-core cluster total | `PCPU`, `PACC0_CPU`, `PACC1_CPU` | `PCPU` |
| E-core manager | `ECPM`, `EACC_CPM` | `MCPM0`, `MCPM1` |
| P-core manager | `PCPM`, `PACC0_CPM` | `PCPM` |
| CPU total | `CPU Energy` | `CPU Energy` |
| DRAM | `DRAM`, `DRAM0` | `DRAM` |
| GPU | `GPU Energy` | `GPU Energy` |
| GPU SRAM | `GPU SRAM`, `GPU SRAM0` | (not available) |
| ANE | `ANE`, `ANE0` | `ANE` |

### 4. Add tests

Add a test case with mock channel data for the new chip in both:
- `tests/tests_cpp/test_monitor.cpp` (e.g., `test_m5_max_example`)
- `tests/tests_python/test_monitor.py` (e.g., `test_m5_max_channels`)

Use the `dump_channels` output to construct realistic mock data. Include channels that should be recognized *and* channels that should be ignored (SRAM, DTL) to verify they don't produce false matches.

### 5. Verify

```bash
cmake --build build
./build/bin/test_monitor                          # C++ tests
python -m pytest tests/tests_python/test_monitor.py  # Python tests
```

Ensure all existing chip tests still pass (no regressions).

### Tested chips

The following chips have test cases in the test suite:

| Chip | E-core naming | P-core naming |
|---|---|---|
| M1 Max | `EACC_CPU*` | `PACC*_CPU*` |
| M3 Pro | `ECPU*` | `PCPU*` |
| M4 | `ECPU*` | `PCPU*` |
| M4 Pro | `EACC_CPU*` | `PACC*_CPU*` |
| M5 Max | `MCPU*_*` | `PACC_*` |


## Source Code Directory Structure

*   `apple_energy/`: Contains the core C++ header library file (`apple_energy.hpp`).
*   `bindings/`: Contains nanobind bindings to create the Python package from the C++ library.
*   `examples/`: Contains sample usage and compilation examples (like the ones above), plus `dump_channels` for inspecting raw IOReport channels on new hardware.
*   `scripts/`: Utility scripts for development and CI.
*   `tests/`: Contains tests, which use mocked data.
