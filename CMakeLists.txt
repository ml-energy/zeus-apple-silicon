cmake_minimum_required(VERSION 3.15...3.27)
project(zeus-apple-silicon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (CMAKE_VERSION VERSION_LESS 3.18)
    set(DEV_MODULE Development)
else()
    set(DEV_MODULE Development.Module)
endif()

# Search for Python >= 3.9.
find_package(Python 3.9 COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

# The lines below configure CMake to perform an optimized release build by
# default unless another build type is specified. Without this addition,
# binding code may run slowly and produce large binaries.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Detect the installed nanobind package and import it into CMake.
execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT
)
find_package(nanobind CONFIG REQUIRED)

# For any files that might use `energy.hpp`.
set(ENERGY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/zeus_apple_silicon)

add_subdirectory(bindings)
add_subdirectory(tests/cpp)
