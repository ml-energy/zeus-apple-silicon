#include "energy_bindings.hpp"
#include <nanobind/nanobind.h>

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(zeus_ext, m)
{
    m.doc() = "An API for programmatically measuring energy consumption on Apple "
              "silicon chips.";

    register_metrics(m);
    register_monitor(m);
}
