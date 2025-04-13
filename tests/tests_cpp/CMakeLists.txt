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
    ${ENERGY_INCLUDE_DIR}
    ${MOCKER_INCLUDE_DIR}
)
