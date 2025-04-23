# Examples

This directory contains examples of how to use the Apple Energy Monitor.

## C++ Examples: `examples/cpp` directory

To compile these programs manually, run:
`g++ your_code.cpp -o your_executable -std=c++17 -framework CoreFoundation -lIOReport`

Where `your_code.cpp` and `your_executable` are replaced with the desired names.

You can also compile these files using CMake. The `CMakeLists.txt` file in the `examples/cpp` directory contains examples of how you can do that. For example:

```CMake
set(
    EXAMPLE_ONE_INTERVAL_SOURCES
    one_interval.cpp
)
add_executable(one_interval
    ${EXAMPLE_ONE_INTERVAL_SOURCES}
)
target_link_libraries(one_interval PRIVATE
    IOReport
    "-framework CoreFoundation"
)
target_include_directories(one_interval PUBLIC ${APPLE_ENERGY_DIR})
```

The last line: `target_include_directories(one_interval PUBLIC ${APPLE_ENERGY_DIR})` is a directive that allows the `apple_energy.hpp` library header file to be visible from within the target example program. `${APPLE_ENERGY_DIR}` is defined in the CMake file at the root of this project. However, if the header file is in the same directory as the program you're including it from, you don't need to expose it via CMake like this.

Also, in the CMake configuration above, note that compilation required linking with IOReport and "-framework CoreFoundation". In general, in order to compile a program that includes the "apple_energy.hpp" library file, you must link your executable with these dependencies.
