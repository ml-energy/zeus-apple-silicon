# Create bindings for an AppleEnergyMonitor backed by
# `ioreport_mocker.cpp` instead of the real IOReport library.
nanobind_add_module(mocked_zeus_ext STABLE_ABI
    mocked_zeus_ext.cpp
    ${MOCKER_SOURCES}
)
target_link_libraries(mocked_zeus_ext PRIVATE
    "-framework CoreFoundation"
)
target_include_directories(mocked_zeus_ext PUBLIC
    ${APPLE_ENERGY_DIR}
    ${ENERGY_BINDINGS_DIR}
    ${MOCKER_DIR}
)
