cmake_minimum_required(VERSION 3.15...3.27)
project(zeus-apple-silicon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (CMAKE_VERSION VERSION_LESS 3.18)
    set(DEV_MODULE Development)
else()
    set(DEV_MODULE Development.Module)
endif()

# Search for Python >= 3.9.
find_package(
    Python 3.9 REQUIRED
    COMPONENTS Interpreter ${DEV_MODULE}
)
# NOTE: if your development environment is VSCode, it's possible that VSCode
# will reconfigure CMake every time on boot-up. The problem is, if you're also
# using a Python virtual environment, VSCode will configure CMake *without* having
# activated your virtual environment. This will cause CMake to find a different
# Python installation in the line above (likely, your system Python).
# If that happens and you want to force CMake to use your virtual environment's
# Python, you must manually reconfigure CMake with your virtual environment
# activated. To do this, first activate your virtual environment, and then
# run the following command from the root of this repository:
#   cmake -S . -B build --fresh
# Note that the `--fresh` flag prevents CMake from using the cached location of
# Python (i.e., the system Python it found earlier).


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

# Define where the `apple_energy.hpp` header file is located.
set(APPLE_ENERGY_DIR ${PROJECT_SOURCE_DIR}/apple_energy)

# Define where the `energy_bindings.hpp` shared bindings header file is located.
set(ENERGY_BINDINGS_DIR ${PROJECT_SOURCE_DIR}/bindings)

if (SKBUILD)
    message(STATUS "Building as scikit-build; only compiling bindings.")
    add_subdirectory(bindings)
    # Install directive for scikit-build-core
    install(TARGETS zeus_ext LIBRARY DESTINATION zeus_apple_silicon)
else()
    message(STATUS "Building normally (not as scikit-build).")
    add_subdirectory(bindings)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()
