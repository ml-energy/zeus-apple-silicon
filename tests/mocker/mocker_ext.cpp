#include <string>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include "mock_ioreport.hpp"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(mocker_ext, m)
{
    m.doc() = "An interface for specifying mock testing data.";

    nb::class_<Mocker>(m, "Mocker")
        .def(nb::init<>())
        .def("push_back_sample", &Mocker::push_back_sample, "data"_a)
        .def("pop_back_sample", &Mocker::pop_back_sample)
        .def("clear_all_mock_samples", &Mocker::clear_all_mock_samples)
        .def("set_sample_index", &Mocker::set_sample_index, "index"_a);
}
