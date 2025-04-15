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

1.  Start a measurement window: Use `AppleEnergyMonitor::begin_window(label)` to start measuring energy consumption. Each window needs a string label passed in as an argument.
2.  Execute Code: Run the code you want to profile.
3.  End & Retrieve: Use `AppleEnergyMonitor::end_window(label)` with the *same label* that started the window to stop measurement for that window. This function returns an `AppleEnergyMetrics` object containing energy data collected during the window.

**Things to Note:**

*   You can have multiple windows active simultaneously (i.e., they can overlap), as long as each uses a distinct label.
*   Non-overlapping windows can re-use names. I.e., once a window is ended with `end_window`, its label is free to be reused.
*   Attempting to start a window with a label still currently in use (i.e., `end_window` not yet called for that label) will raise an exception.
*   Calling `end_window` with a label that doesn't belong to any currently active window will raise an exception.


## Usage Examples

The API is identical in C++ and Python.

### C++ Example

```C++
#include "apple_energy.hpp"

int main() {
    // Create a monitor instance.
    AppleEnergyMonitor monitor;

    // --- Basic Measurement ---
    monitor.begin_window("task_1");

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
monitor.begin_window("task_1")

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
*   `begin_window(label: str)`: Starts a new measurement window identified by `label`.
*   `end_window(label: str) -> AppleEnergyMetrics`: Ends the measurement window identified by `label` and returns an object containing the results.
*   `get_cumulative_energy() -> AppleEnergyMetrics`: Returns cumulative energy consumed from an unspecified point fixed over the lifetime of the energy monitor (e.g., from bootup).

### Struct/Class: `AppleEnergyMetrics`

This struct/class is how results get reported, containing metrics for various different SoC subsystems.

All energy values are reported in mJ.

Note: on some hardware configurations or macOS versions, certain metrics might not be available or reportable by the underlying system. In such cases, the corresponding attribute will be:
*   C++: An empty `std::optional`. You should check `.has_value()` before accessing `.value()`.
*   Python: `None`. You should check for `None` before using the value.

**Fields Reported in AppleEnergyMetrics:**

*   **CPU Related Metrics:**
    *   `cpu_total_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Total energy consumed by all CPU related subsystems combined.
    *   `efficiency_cores_mj`: `std::optional<std::vector<int64_t>>` (C++) / `Optional[list[int]]` (Python)
        *   Energy consumed by each efficiency core individually. Returns a list where each element corresponds to an efficiency core.
    *   `performance_cores_mj`: `std::optional<std::vector<int64_t>>` (C++) / `Optional[list[int]]` (Python)
        *   Energy consumed by each performance core individually. Returns a list where each element corresponds to a performance core.
    *   `efficiency_core_manager_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy attributed to the efficiency core cluster's management logic.
    *   `performance_core_manager_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy attributed to the performance core cluster's management logic.

*   **DRAM Metrics:**
    *   `dram_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy consumed by DRAM.

*   **GPU Related Metrics:**
    *   `gpu_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy consumed by the on-chip GPU.
    *   `gpu_sram_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy consumed by the GPU's SRAM.

*   **ANE Metrics:**
    *   `ane_mj`: `std::optional<int64_t>` (C++) / `Optional[int]` (Python)
        *   Energy consumed by the Apple Neural Engine (ANE).


## Source Code Directory Structure

*   `apple_energy/`: Contains the core C++ header library file (`apple_energy.hpp`).
*   `bindings/`: Contains nanobind bindings to create the Python package from the C++ library.
*   `examples/`: Contains sample usage and compilation examples (like the ones above).
*   `scripts/`: Utility scripts for development and CI.
*   `tests/`: Contains tests, which use mocked data.
