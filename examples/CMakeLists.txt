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
