"""Check Apple Silicon hardware compatibility with zeus-apple-silicon.

Run this script on a new Apple Silicon machine to verify that the IOReport
energy monitoring interface works and to inspect what energy channels the
specific chip exports.

Usage:
    pip install zeus-apple-silicon
    python check_compatibility.py
"""

from __future__ import annotations

import platform
import subprocess
import time

from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics


def get_chip_info() -> dict[str, str]:
    """Collect Apple Silicon chip information via sysctl."""
    info: dict[str, str] = {}
    keys = {
        "machdep.cpu.brand_string": "CPU",
        "hw.ncpu": "Total cores",
        "hw.perflevel0.logicalcpu": "P-cores",
        "hw.perflevel1.logicalcpu": "E-cores",
        "hw.memsize": "Memory (bytes)",
        "hw.nperflevels": "Performance levels",
    }
    for sysctl_key, label in keys.items():
        try:
            result = subprocess.run(
                ["sysctl", "-n", sysctl_key],
                capture_output=True,
                text=True,
                check=True,
            )
            info[label] = result.stdout.strip()
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass
    return info


def format_field(name: str, value: int | list[int] | None, unit: str = "mJ") -> str:
    """Format a single metric field for display."""
    if value is None:
        return f"  {name}: not available"
    if isinstance(value, list):
        items = ", ".join(f"{v} {unit}" for v in value)
        return f"  {name}: [{items}] ({len(value)} cores)"
    return f"  {name}: {value} {unit}"


def print_metrics(label: str, metrics: AppleEnergyMetrics) -> None:
    """Print all fields of an AppleEnergyMetrics object."""
    print(f"\n{label}:")
    print(format_field("CPU total", metrics.cpu_total_mj))
    print(format_field("Efficiency cores", metrics.efficiency_cores_mj))
    print(format_field("Efficiency core manager", metrics.efficiency_core_manager_mj))
    print(format_field("Performance cores", metrics.performance_cores_mj))
    print(format_field("Performance core manager", metrics.performance_core_manager_mj))
    print(format_field("DRAM", metrics.dram_mj))
    print(format_field("GPU", metrics.gpu_mj))
    print(format_field("GPU SRAM", metrics.gpu_sram_mj))
    print(format_field("ANE", metrics.ane_mj))


def summarize_available(metrics: AppleEnergyMetrics) -> tuple[list[str], list[str]]:
    """Return lists of available and unavailable metric names."""
    fields = {
        "cpu_total_mj": metrics.cpu_total_mj,
        "efficiency_cores_mj": metrics.efficiency_cores_mj,
        "efficiency_core_manager_mj": metrics.efficiency_core_manager_mj,
        "performance_cores_mj": metrics.performance_cores_mj,
        "performance_core_manager_mj": metrics.performance_core_manager_mj,
        "dram_mj": metrics.dram_mj,
        "gpu_mj": metrics.gpu_mj,
        "gpu_sram_mj": metrics.gpu_sram_mj,
        "ane_mj": metrics.ane_mj,
    }
    available = [name for name, val in fields.items() if val is not None]
    unavailable = [name for name, val in fields.items() if val is None]
    return available, unavailable


def cpu_workload(duration_seconds: float = 2.0) -> None:
    """Burn CPU for roughly `duration_seconds`."""
    end = time.monotonic() + duration_seconds
    x = 0
    while time.monotonic() < end:
        for i in range(100_000):
            x += i
    _ = x


def main() -> None:
    # ---- System info ----
    print("=" * 60)
    print("Zeus Apple Silicon Compatibility Check")
    print("=" * 60)

    print(f"\nPython: {platform.python_version()}")
    print(f"macOS:  {platform.mac_ver()[0]}")
    print(f"Arch:   {platform.machine()}")

    chip_info = get_chip_info()
    if chip_info:
        print("\nChip info (sysctl):")
        for label, value in chip_info.items():
            if label == "Memory (bytes)":
                gb = int(value) / (1024**3)
                print(f"  {label}: {value} ({gb:.0f} GB)")
            else:
                print(f"  {label}: {value}")

    # ---- Initialize monitor ----
    print("\n" + "-" * 60)
    print("Initializing AppleEnergyMonitor...")
    try:
        monitor = AppleEnergyMonitor()
    except Exception as e:
        print(f"FAILED: {e}")
        print("\nThis chip may not be compatible with IOReport energy monitoring.")
        return
    print("OK")

    # ---- Cumulative energy snapshot ----
    print("\n" + "-" * 60)
    print("Reading cumulative energy snapshot...")
    cumulative = monitor.get_cumulative_energy()
    print_metrics("Cumulative energy (system-wide since boot)", cumulative)

    available, unavailable = summarize_available(cumulative)
    print(f"\nAvailable metrics ({len(available)}/{len(available) + len(unavailable)}):")
    for name in available:
        print(f"  + {name}")
    if unavailable:
        print(f"Unavailable metrics ({len(unavailable)}):")
        for name in unavailable:
            print(f"  - {name}")

    # ---- Measure a CPU workload ----
    print("\n" + "-" * 60)
    workload_seconds = 2.0
    print(f"Measuring a {workload_seconds:.0f}s CPU workload...")

    monitor.begin_window("cpu_workload")
    cpu_workload(workload_seconds)
    measurement = monitor.end_window("cpu_workload")

    print_metrics("CPU workload measurement", measurement)

    # Sanity check: at least CPU total should be non-zero.
    if measurement.cpu_total_mj is not None and measurement.cpu_total_mj > 0:
        print("\nCPU energy reading: OK (non-zero)")
    elif measurement.cpu_total_mj is not None:
        print("\nWARNING: CPU total energy is zero after workload.")
    else:
        print("\nWARNING: CPU total energy is not available on this chip.")

    # ---- Test restart feature ----
    print("\n" + "-" * 60)
    print("Testing begin_window restart feature...")

    monitor.begin_window("restart_test")
    try:
        monitor.begin_window("restart_test")
        print("FAILED: expected error for duplicate window without restart=True")
    except RuntimeError:
        print("  Duplicate window correctly rejected: OK")

    monitor.begin_window("restart_test", restart=True)
    result = monitor.end_window("restart_test")
    print("  Restart and end_window succeeded: OK")

    # ---- Summary ----
    print("\n" + "=" * 60)
    print("Compatibility check complete.")

    core_info_parts = []
    if measurement.efficiency_cores_mj is not None:
        core_info_parts.append(f"{len(measurement.efficiency_cores_mj)} E-cores")
    if measurement.performance_cores_mj is not None:
        core_info_parts.append(f"{len(measurement.performance_cores_mj)} P-cores")
    if core_info_parts:
        print(f"Core topology (from energy channels): {', '.join(core_info_parts)}")

    print(f"Available metrics: {len(available)}/{len(available) + len(unavailable)}")

    if measurement.cpu_total_mj is not None and measurement.cpu_total_mj > 0:
        print("Status: PASS")
    else:
        print("Status: NEEDS INVESTIGATION (see warnings above)")


if __name__ == "__main__":
    main()
