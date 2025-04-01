set(
    TEST_BASIC_SOURCES
    test_basic.cpp
)

add_executable(test_basic ${TEST_BASIC_SOURCES})
target_link_libraries(test_basic PRIVATE
    IOReport                       # If compiling manually, this corresponds to: -lIOReport
    "-framework CoreFoundation"    # And this corresponds to: -framework CoreFoundation
)
target_include_directories(test_basic PUBLIC ${ENERGY_INCLUDE_DIR})
