set(
    TEST_MONITOR_SOURCES
    test_monitor.cpp
)

add_executable(test_monitor
    ${TEST_MONITOR_SOURCES}
    ${MOCKER_SOURCES}
)
target_link_libraries(test_monitor PRIVATE
    "-framework CoreFoundation"
)

target_include_directories(test_monitor PUBLIC
    ${APPLE_ENERGY_DIR}
    ${MOCKER_DIR}
)

# Ensure assert statements work by building in debug.
target_compile_options(test_monitor PRIVATE -O0 -g)
target_compile_definitions(test_monitor PRIVATE DEBUG)
