#include <string>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/optional.h>

#include "energy.hpp"

namespace nb = nanobind;
using namespace nb::literals;

std::string represent_metrics(const AppleEnergyMetrics& metrics) {
    std::string repr = "";

    repr += "CPU: ";
    repr += metrics.cpu_mj ? (
        std::to_string(metrics.cpu_mj.value()) + " mJ\n"
    ) : "None (unavailable)\n";

    repr += "GPU: ";
    repr += metrics.gpu_mj ? (
        std::to_string(metrics.gpu_mj.value()) + " mJ\n"
    ) : "None (unavailable)\n";

    repr += "ANE: ";
    repr += metrics.ane_mj ? (
        std::to_string(metrics.ane_mj.value()) + " mJ\n"
    ) : "None (unavailable)\n";

    repr += "DRAM: ";
    repr += metrics.dram_mj ? (
        std::to_string(metrics.dram_mj.value()) + " mJ\n"
    ) : "None (unavailable)\n";

    repr += "GPU_SRAM: ";
    repr += metrics.gpu_sram_mj ? (
        std::to_string(metrics.gpu_sram_mj.value()) + " mJ\n"
    ) : "None (unavailable)\n";

    return repr;
}

NB_MODULE(zeus_ext, m) {
    m.doc() = "An API for programmatically measuring energy consumption on Apple silicon chips.";

    nb::class_<AppleEnergyMetrics>(m, "AppleEnergyMetrics")
        .def(nb::init<>())
        .def("__repr__", &represent_metrics)
        .def_rw("cpu_mj", &AppleEnergyMetrics::cpu_mj)
        .def_rw("gpu_mj", &AppleEnergyMetrics::gpu_mj)
        .def_rw("ane_mj", &AppleEnergyMetrics::ane_mj)
        .def_rw("dram_mj", &AppleEnergyMetrics::dram_mj)
        .def_rw("gpu_sram_mj", &AppleEnergyMetrics::gpu_sram_mj);

    nb::class_<AppleEnergyMonitor>(m, "AppleEnergyMonitor")
        .def(nb::init<>())
        .def("get_cumulative_energy", &AppleEnergyMonitor::get_cumulative_energy)
        .def("begin_window", &AppleEnergyMonitor::begin_window, "key"_a)
        .def("end_window", &AppleEnergyMonitor::end_window, "key"_a);
}
