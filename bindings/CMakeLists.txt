nanobind_add_module(zeus_ext zeus_ext.cpp)
target_link_libraries(zeus_ext PRIVATE
    IOReport                       # If compiling manually, this corresponds to: -lIOReport
    "-framework CoreFoundation"    # And this corresponds to: -framework CoreFoundation
)
target_include_directories(zeus_ext PUBLIC ${ENERGY_INCLUDE_DIR})
