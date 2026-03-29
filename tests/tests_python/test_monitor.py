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


def test_restart_window():
    """Test that begin_window with restart=True cancels and restarts an existing window."""
    mocker = Mocker()
    # Sample for first begin_window
    mocker.push_back_sample(
        {
            "CPU Energy": (10000000, "mJ"),
            "GPU Energy": (1000000, "nJ"),
        }
    )
    # Sample for restarted begin_window
    mocker.push_back_sample(
        {
            "CPU Energy": (20000000, "mJ"),
            "GPU Energy": (2000000, "nJ"),
        }
    )
    # Sample for end_window
    mocker.push_back_sample(
        {
            "CPU Energy": (50000000, "mJ"),
            "GPU Energy": (5000000, "nJ"),
        }
    )

    monitor = AppleEnergyMonitor()

    monitor.begin_window("test")

    # Without restart, should raise.
    with pytest.raises(RuntimeError):
        monitor.begin_window("test")

    # With restart=True, should succeed.
    monitor.begin_window("test", restart=True)

    result = monitor.end_window("test")
    # Delta is from the restarted sample (20M) to end sample (50M).
    assert result.cpu_total_mj == 30000000
    assert result.gpu_mj == 3

    # restart=True on a non-existent window should just start normally.
    mocker.push_back_sample(
        {
            "CPU Energy": (0, "mJ"),
            "GPU Energy": (0, "nJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (100, "mJ"),
            "GPU Energy": (1000000, "nJ"),
        }
    )
    monitor.begin_window("new_key", restart=True)
    result2 = monitor.end_window("new_key")
    assert result2.cpu_total_mj == 100


def test_m5_max_channels():
    """Test M5 Max channel naming: MCPUx_y for E-cores, PACC_y for P-cores, MCPMx for E-core managers."""
    mocker = Mocker()

    data1 = {
        # E-core cluster 0
        "MCPU0_0": (0, "mJ"),
        "MCPU0_1": (0, "mJ"),
        "MCPU0_2": (0, "mJ"),
        "MCPU0_3": (0, "mJ"),
        "MCPU0_4": (0, "mJ"),
        "MCPU0_5": (0, "mJ"),
        # E-core cluster 1
        "MCPU1_0": (0, "mJ"),
        "MCPU1_1": (0, "mJ"),
        "MCPU1_2": (0, "mJ"),
        "MCPU1_3": (0, "mJ"),
        "MCPU1_4": (0, "mJ"),
        "MCPU1_5": (0, "mJ"),
        # P-cores
        "PACC_0": (0, "mJ"),
        "PACC_1": (0, "mJ"),
        "PACC_2": (0, "mJ"),
        "PACC_3": (0, "mJ"),
        "PACC_4": (0, "mJ"),
        "PACC_5": (0, "mJ"),
        # E-core managers
        "MCPM0": (0, "mJ"),
        "MCPM1": (0, "mJ"),
        # P-core manager
        "PCPM": (0, "mJ"),
        # Cluster totals
        "MCPU0": (0, "mJ"),
        "MCPU1": (0, "mJ"),
        "PCPU": (0, "mJ"),
        # SRAM/DTL channels (should NOT be parsed as cores)
        "MCPU0_0_SRAM": (0, "mJ"),
        "PCPU0_SRAM": (0, "mJ"),
        "MCPU0DTL00": (0, "mJ"),
        "PCPUDTL00": (0, "mJ"),
        # System
        "CPU Energy": (0, "mJ"),
        "DRAM": (0, "mJ"),
        "GPU Energy": (0, "nJ"),
        "ANE": (0, "mJ"),
    }
    mocker.push_back_sample(data1)

    data2 = {
        "MCPU0_0": (100, "mJ"),
        "MCPU0_1": (90, "mJ"),
        "MCPU0_2": (80, "mJ"),
        "MCPU0_3": (70, "mJ"),
        "MCPU0_4": (60, "mJ"),
        "MCPU0_5": (50, "mJ"),
        "MCPU1_0": (95, "mJ"),
        "MCPU1_1": (85, "mJ"),
        "MCPU1_2": (75, "mJ"),
        "MCPU1_3": (65, "mJ"),
        "MCPU1_4": (55, "mJ"),
        "MCPU1_5": (45, "mJ"),
        "PACC_0": (500, "mJ"),
        "PACC_1": (400, "mJ"),
        "PACC_2": (300, "mJ"),
        "PACC_3": (250, "mJ"),
        "PACC_4": (200, "mJ"),
        "PACC_5": (150, "mJ"),
        "MCPM0": (30, "mJ"),
        "MCPM1": (25, "mJ"),
        "PCPM": (40, "mJ"),
        "MCPU0": (9999, "mJ"),
        "MCPU1": (8888, "mJ"),
        "PCPU": (7777, "mJ"),
        "MCPU0_0_SRAM": (10, "mJ"),
        "PCPU0_SRAM": (20, "mJ"),
        "MCPU0DTL00": (5, "mJ"),
        "PCPUDTL00": (6, "mJ"),
        "CPU Energy": (2000, "mJ"),
        "DRAM": (100, "mJ"),
        "GPU Energy": (5000000, "nJ"),
        "ANE": (10, "mJ"),
    }
    mocker.push_back_sample(data2)

    monitor = AppleEnergyMonitor()

    monitor.begin_window("test")
    result = monitor.end_window("test")

    assert result.cpu_total_mj == 2000

    # 12 E-cores (two clusters of 6).
    assert result.efficiency_cores_mj is not None
    assert len(result.efficiency_cores_mj) == 12
    assert sorted(result.efficiency_cores_mj) == [
        45,
        50,
        55,
        60,
        65,
        70,
        75,
        80,
        85,
        90,
        95,
        100,
    ]

    # 6 P-cores.
    assert result.performance_cores_mj is not None
    assert len(result.performance_cores_mj) == 6
    assert sorted(result.performance_cores_mj) == [150, 200, 250, 300, 400, 500]

    # E-core cluster totals: MCPU0=9999, MCPU1=8888.
    assert result.efficiency_cluster_mj is not None
    assert len(result.efficiency_cluster_mj) == 2
    assert sorted(result.efficiency_cluster_mj) == [8888, 9999]

    # P-core cluster total: PCPU=7777.
    assert result.performance_cluster_mj is not None
    assert result.performance_cluster_mj == [7777]

    # E-core manager: MCPM0 + MCPM1 = 55.
    assert result.efficiency_core_manager_mj == 55
    # P-core manager: PCPM = 40.
    assert result.performance_core_manager_mj == 40

    assert result.dram_mj == 100
    assert result.gpu_mj == 5
    assert result.ane_mj == 10

    # GPU SRAM should still be None (no "GPU SRAM" channel on M5).
    assert result.gpu_sram_mj is None


def test_some_fields_none():
    mocker = Mocker()
    mocker.push_back_sample(
        {
            "CPU Energy": (0, "mJ"),
            "GPU Dummy": (0, "nJ"),
            "EC0": (0, "mJ"),
            "EC1": (0, "mJ"),
            "ECPM": (0, "mJ"),
            "PCPU0": (0, "mJ"),
            "PCPU1": (0, "mJ"),
            "PCPM": (0, "mJ"),
            "GPU SRAM": (0, "mJ"),
        }
    )
    mocker.push_back_sample(
        {
            "CPU Energy": (10000000, "mJ"),
            "GPU Dummy": (1000000, "nJ"),
            "EC0": (10000, "mJ"),
            "EC1": (10000, "mJ"),
            "ECPM": (10000, "mJ"),
            "PCPU0": (90000, "mJ"),
            "PC1": (10000, "mJ"),
            "PCPM": (10000, "mJ"),
            "GPU SRAM": (10000, "mJ"),
        }
    )

    mon = AppleEnergyMonitor()
    mon.begin_window("test")
    res = mon.end_window("test")

    assert isinstance(res, AppleEnergyMetrics)
    assert res.cpu_total_mj == 10000000
    assert res.gpu_mj is None
    assert res.efficiency_cores_mj is None
    assert res.efficiency_core_manager_mj == 10000
    assert res.performance_cores_mj == [90000]
    assert res.performance_core_manager_mj == 10000
    assert res.dram_mj is None
    assert res.gpu_sram_mj == 10000
    assert res.ane_mj is None
