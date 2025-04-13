# Create bindings for an AppleEnergyMonitor backed by
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
target_compile_definitions(mocked_zeus_ext PRIVATE MOCK_MODE=1)
target_include_directories(mocked_zeus_ext PUBLIC ${MOCKER_INCLUDE_DIR})
