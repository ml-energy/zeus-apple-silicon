#ifndef EXT_NAME
#define EXT_NAME "zeus_ext"
#endif

#include <string>

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include "energy.hpp"

namespace nb = nanobind;
using namespace nb::literals;

std::string represent_metrics(const AppleEnergyMetrics& metrics)
{
    std::string repr = "";

    repr += "CPU Total: ";
    repr += metrics.cpu_total_mj
        ? (std::to_string(metrics.cpu_total_mj.value()) + " mJ\n")
        : "None (unavailable)\n";

    repr += "Efficiency cores: ";
    if (metrics.efficiency_cores_mj) {
        for (const auto& core : metrics.efficiency_cores_mj.value()) {
            repr += std::to_string(core) + " mJ  ";
        }
        repr += "\n";
    } else {
        repr += "None (unavailable)\n";
    }

    repr += "Performance cores: ";
    if (metrics.performance_cores_mj) {
        for (const auto& core : metrics.performance_cores_mj.value()) {
            repr += std::to_string(core) + " mJ  ";
        }
        repr += "\n";
    } else {
        repr += "None (unavailable)\n";
    }

    repr += "Efficiency core manager: ";
    repr += metrics.efficiency_core_manager_mj
        ? (std::to_string(metrics.efficiency_core_manager_mj.value()) + " mJ\n")
        : "None (unavailable)\n";

    repr += "Performance core manager: ";
    repr += metrics.performance_core_manager_mj
        ? (std::to_string(metrics.performance_core_manager_mj.value()) + " mJ\n")
        : "None (unavailable)\n";

    repr += "DRAM: ";
    repr += metrics.dram_mj ? (std::to_string(metrics.dram_mj.value()) + " mJ\n")
                            : "None (unavailable)\n";
    repr += "GPU: ";
    repr += metrics.gpu_mj ? (std::to_string(metrics.gpu_mj.value()) + " mJ\n")
                           : "None (unavailable)\n";
    repr += "GPU SRAM: ";
    repr += metrics.gpu_sram_mj
        ? (std::to_string(metrics.gpu_sram_mj.value()) + " mJ\n")
        : "None (unavailable)\n";
    repr += "ANE: ";
    repr += metrics.ane_mj ? (std::to_string(metrics.ane_mj.value()) + " mJ\n")
                           : "None (unavailable)\n";

    return repr;
}

NB_MODULE(EXT_NAME, m)
{
    m.doc() = "An API for programmatically measuring energy consumption on Apple "
              "silicon chips.";

    nb::class_<AppleEnergyMetrics>(m, "AppleEnergyMetrics")
        .def(nb::init<>())
        .def("__repr__", &represent_metrics)
        .def_rw("cpu_total_mj", &AppleEnergyMetrics::cpu_total_mj)
        .def_rw("efficiency_cores_mj", &AppleEnergyMetrics::efficiency_cores_mj)
        .def_rw("performance_cores_mj", &AppleEnergyMetrics::performance_cores_mj)
        .def_rw("efficiency_core_manager_mj",
            &AppleEnergyMetrics::efficiency_core_manager_mj)
        .def_rw("performance_core_manager_mj",
            &AppleEnergyMetrics::performance_core_manager_mj)
        .def_rw("dram_mj", &AppleEnergyMetrics::dram_mj)
        .def_rw("gpu_mj", &AppleEnergyMetrics::gpu_mj)
        .def_rw("gpu_sram_mj", &AppleEnergyMetrics::gpu_sram_mj)
        .def_rw("ane_mj", &AppleEnergyMetrics::ane_mj);

    nb::class_<AppleEnergyMonitor>(m, "AppleEnergyMonitor")
        .def(nb::init<>())
        .def("get_cumulative_energy", &AppleEnergyMonitor::get_cumulative_energy)
        .def("begin_window", &AppleEnergyMonitor::begin_window, "key"_a)
        .def("end_window", &AppleEnergyMonitor::end_window, "key"_a);
}
