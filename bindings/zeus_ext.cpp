#include <nanobind/nanobind.h>
#include "energy.hpp"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(zeus_ext, m) {
    m.doc() = "An API for programmatically measuring energy consumption on Apple silicon chips.";

    nb::class_<AppleEnergyMetrics>(m, "AppleEnergyMetrics")
        .def(nb::init<>())
        .def_rw("cpu_mj", &AppleEnergyMetrics::cpu_mj);

    nb::class_<AppleEnergyMonitor>(m, "AppleEnergyMonitor")
        .def(nb::init<>())
        .def("get_cumulative_energy", &AppleEnergyMonitor::get_cumulative_energy);
}
