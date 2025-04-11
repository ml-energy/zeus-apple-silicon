from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics
import pytest


def dummy_work(limit):
    """Dummy work to exercise the machine a little bit."""
    x = 0
    for i in range(limit):
        for j in range(10000):
            x += j


def test_energy_monitor():
    """Simulate a well-formulated usage of the energy monitor."""
    monitor = AppleEnergyMonitor()
    assert isinstance(monitor, AppleEnergyMonitor)

    monitor.begin_window("test1")
    dummy_work(1000)
    result = monitor.end_window("test1")

    assert isinstance(result, AppleEnergyMetrics)

    # Regardless of the specific Apple chip, cpu_total_mj and gpu_mj should generally be non-None.
    assert result.cpu_total_mj is not None
    assert result.gpu_mj is not None


# TODO: add more tests
