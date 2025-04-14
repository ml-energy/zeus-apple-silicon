#include <nanobind/nanobind.h>
#include <nanobind/stl/unordered_map.h>

#include "energy_bindings.hpp"
#include "ioreport_mocker.hpp"

namespace nb = nanobind;
using namespace nb::literals;

void register_mocker(nb::module_& m)
{
    nb::class_<Mocker>(m, "Mocker")
        .def(nb::init<>())
        .def("push_back_sample", &Mocker::push_back_sample, "data"_a)
        .def("pop_back_sample", &Mocker::pop_back_sample)
        .def("clear_all_mock_samples", &Mocker::clear_all_mock_samples)
        .def("set_sample_index", &Mocker::set_sample_index, "index"_a);
}

NB_MODULE(mocked_zeus_ext, m)
{
    m.doc() = "AppleEnergyMonitor using mocked data instead of the real IOReport library";

    register_metrics(m);
    register_monitor(m);
    register_mocker(m);
}
