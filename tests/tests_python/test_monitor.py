from build.tests.mocker.mocked_zeus_ext import AppleEnergyMonitor, AppleEnergyMetrics, Mocker
import pytest


def test_one_interval():
    """Simulate a well-formulated usage of the energy monitor."""

    mocker = Mocker()
    mocker.push_back_sample({
        "CPU Energy": 10000000,
        "GPU Energy": 1000000,
    })
    mocker.push_back_sample({
        "CPU Energy": 90000000,
        "GPU Energy": 6000000,
    })

    monitor = AppleEnergyMonitor()
    assert isinstance(monitor, AppleEnergyMonitor)

    monitor.begin_window("test1")
    result = monitor.end_window("test1")

    assert isinstance(result, AppleEnergyMetrics)
    assert result.cpu_total_mj is not None
    assert result.gpu_mj is not None

    assert result.cpu_total_mj == 80000000
    assert result.gpu_mj == 5


def test_overlapping_intervals():
    """Simulate overlapping intervals."""

    mocker = Mocker()
    mocker.push_back_sample({
        "CPU Energy": 10000000,
        "GPU Energy": 1000000,
    })
    mocker.push_back_sample({
        "CPU Energy": 20000000,
        "GPU Energy": 2000000,
    })
    mocker.push_back_sample({
        "CPU Energy": 50000000,
        "GPU Energy": 5000000,
    })
    mocker.push_back_sample({
        "CPU Energy": 80000000,
        "GPU Energy": 8000000,
    })

    monitor = AppleEnergyMonitor()

    monitor.begin_window("test1")
    monitor.begin_window("test2")

    res2 = monitor.end_window("test2")
    res1 = monitor.end_window("test1")

    assert res1.cpu_total_mj is not None
    assert res2.cpu_total_mj is not None

    assert res1.cpu_total_mj == 70000000
    assert res1.gpu_mj == 7

    assert res2.cpu_total_mj == 30000000
    assert res2.gpu_mj == 3


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
