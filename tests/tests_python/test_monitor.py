from build.tests.mocker.mocked_zeus_ext import AppleEnergyMonitor, AppleEnergyMetrics
from build.tests.mocker.mocker_ext import Mocker
import pytest


# def dummy_work(limit):
#     """Dummy work to exercise the machine a little bit."""
#     x = 0
#     for i in range(limit):
#         for j in range(10000):
#             x += j


# def test_one_interval():
#     """Simulate a well-formulated usage of the energy monitor."""
#     monitor = AppleEnergyMonitor()
#     assert isinstance(monitor, AppleEnergyMonitor)

#     monitor.begin_window("test1")
#     dummy_work(1000)
#     result = monitor.end_window("test1")

#     assert isinstance(result, AppleEnergyMetrics)

#     # Regardless of the specific Apple chip, results for `cpu_total_mj` and
#     # `gpu_mj` should almost certainly be present.
#     assert result.cpu_total_mj is not None
#     assert result.gpu_mj is not None

#     # CPU energy should almost certainly be positive.
#     assert result.cpu_total_mj > 0

#     # GPU energy should be positive or at least non-negative (possible
#     # that it was zero or very close).
#     assert result.gpu_mj >= 0


# def test_overlapping_intervals():
#     """Simulate overlapping intervals."""
#     monitor = AppleEnergyMonitor()

#     monitor.begin_window("test1")
#     dummy_work(1000)

#     monitor.begin_window("test2")
#     dummy_work(1000)

#     res2 = monitor.end_window("test2")
#     res1 = monitor.end_window("test1")

#     assert res1.cpu_total_mj is not None
#     assert res2.cpu_total_mj is not None
#     assert res1.cpu_total_mj > res2.cpu_total_mj


# def test_invalid_keys():
#     """Verify that invalid keys are handled gracefully."""
#     monitor = AppleEnergyMonitor()

#     monitor.begin_window("test1")

#     # Creating a new window with the same key is invalid.
#     with pytest.raises(RuntimeError):
#         monitor.begin_window("test1")

#     # Ending a window that was never started is invalid.
#     with pytest.raises(RuntimeError):
#         monitor.end_window("test2")

#     monitor.end_window("test1")

#     # Now, starting a window with key of "test1" is valid.
#     monitor.begin_window("test1")


# def test_cumulative_energy():
#     """Verify that cumulative metrics are sensibly produced."""
#     monitor = AppleEnergyMonitor()

#     res1 = monitor.get_cumulative_energy()
#     assert isinstance(res1, AppleEnergyMetrics)
#     assert res1.cpu_total_mj is not None
#     assert res1.gpu_mj is not None

#     dummy_work(2000)

#     res2 = monitor.get_cumulative_energy()
#     assert isinstance(res2, AppleEnergyMetrics)
#     assert res2.cpu_total_mj is not None
#     assert res2.gpu_mj is not None

#     assert res2.cpu_total_mj > res1.cpu_total_mj
#     assert res2.gpu_mj >= res1.gpu_mj
