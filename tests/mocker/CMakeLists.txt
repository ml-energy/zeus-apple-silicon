# Create bindings for an AppleEnergyMonitor backed by a
# `mock_ioreport.cpp` instead of the real IOReport library.
nanobind_add_module(mocked_zeus_ext STABLE_ABI
    ${PROJECT_SOURCE_DIR}/bindings/zeus_ext.cpp
    ${MOCKER_SOURCES}
)
target_link_libraries(mocked_zeus_ext PRIVATE
    "-framework CoreFoundation"
)
target_include_directories(mocked_zeus_ext PUBLIC ${ENERGY_INCLUDE_DIR})
target_compile_definitions(mocked_zeus_ext PRIVATE EXT_NAME=mocked_zeus_ext)


# Create bindings for an interface allowing mock data to be passed to the mocker.
nanobind_add_module(mocker_ext STABLE_ABI
    mocker_ext.cpp
    ${MOCKER_SOURCES}
)
target_link_libraries(mocker_ext PRIVATE
    "-framework CoreFoundation"
)
target_include_directories(mocker_ext PUBLIC ${MOCKER_INCLUDE_DIR})
