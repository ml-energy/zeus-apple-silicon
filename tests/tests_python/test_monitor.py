from build.tests.mocker.mocked_zeus_ext import (
    AppleEnergyMonitor,
    AppleEnergyMetrics,
    Mocker,
)
import pytest


def test_one_interval():
    """Simulate a well-formulated usage of the energy monitor."""

    mocker = Mocker()
    mocker.push_back_sample(
        {
            "CPU Energy": (10000000, "mJ"),
            "GPU Energy": (1000000, "nJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (90000000, "mJ"),
            "GPU Energy": (6000000, "nJ"),
        }
    )

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
    mocker.push_back_sample(
        {
            "CPU Energy": (10000000, "mJ"),
            "GPU Energy": (1000000, "nJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (20000000, "mJ"),
            "GPU Energy": (2000000, "nJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (50000000, "mJ"),
            "GPU Energy": (5000000, "nJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (80000000, "mJ"),
            "GPU Energy": (8000000, "nJ"),
        }
    )

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


def test_invalid_keys():
    """Verify that invalid keys are handled gracefully."""
    mocker = Mocker()

    # Three samples will be drawn from the mocker.
    for _ in range(3):
        mocker.push_back_sample({})

    monitor = AppleEnergyMonitor()

    monitor.begin_window("test1")

    # Creating a new window with the same key is invalid.
    with pytest.raises(RuntimeError):
        monitor.begin_window("test1")

    # Ending a window that was never started is invalid.
    with pytest.raises(RuntimeError):
        monitor.end_window("test2")

    monitor.end_window("test1")

    # Now, starting a window with key of "test1" is valid.
    monitor.begin_window("test1")


def test_cumulative_energy():
    """Verify that cumulative metrics are sensibly produced."""
    mocker = Mocker()
    monitor = AppleEnergyMonitor()

    mocker.push_back_sample(
        {
            "CPU Energy": (100, "J"),
            "GPU Energy": (100, "kJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (100, "J"),
            "GPU Energy": (100, "kJ"),
        }
    )

    res1 = monitor.get_cumulative_energy()
    assert isinstance(res1, AppleEnergyMetrics)
    assert res1.cpu_total_mj == 100000
    assert res1.gpu_mj == 100000000


def test_all_fields():
    mocker = Mocker()
    mocker.push_back_sample(
        {
            "CPU Energy": (10000000, "mJ"),
            "GPU Energy": (1000000, "nJ"),
            "ECPU0": (10000, "mJ"),
            "ECPU1": (10000, "mJ"),
            "ECPM": (10000, "mJ"),
            "PCPU0": (10000, "mJ"),
            "PCPU1": (10000, "mJ"),
            "PCPM": (10000, "mJ"),
            "DRAM": (10000, "mJ"),
            "GPU SRAM": (10000, "mJ"),
            "ANE": (10000, "mJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (30000000, "mJ"),
            "GPU Energy": (3000000, "nJ"),
            "ECPU0": (10001, "mJ"),
            "ECPU1": (10001, "mJ"),
            "ECPM": (10002, "mJ"),
            "PCPU0": (10003, "mJ"),
            "PCPU1": (10003, "mJ"),
            "PCPM": (10004, "mJ"),
            "DRAM": (10005, "mJ"),
            "GPU SRAM": (10006, "mJ"),
            "ANE": (10007, "mJ"),
        }
    )

    mon = AppleEnergyMonitor()

    mon.begin_window("test")
    res = mon.end_window("test")

    assert isinstance(res, AppleEnergyMetrics)
    assert res.cpu_total_mj == 20000000
    assert res.gpu_mj == 2
    assert res.efficiency_cores_mj == [1, 1]
    assert res.efficiency_core_manager_mj == 2
    assert res.performance_cores_mj == [3, 3]
    assert res.performance_core_manager_mj == 4
    assert res.dram_mj == 5
    assert res.gpu_sram_mj == 6
    assert res.ane_mj == 7
