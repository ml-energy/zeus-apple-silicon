nanobind_add_module(zeus_ext STABLE_ABI zeus_ext.cpp)
target_link_libraries(zeus_ext PRIVATE
    IOReport                       # If compiling manually, this corresponds to: -lIOReport
    "-framework CoreFoundation"    # And this corresponds to: -framework CoreFoundation
)
target_include_directories(zeus_ext PUBLIC
    ${APPLE_ENERGY_DIR}     # apple_energy.hpp
    ${ENERGY_BINDINGS_DIR}  # energy_bindings.hpp
)
