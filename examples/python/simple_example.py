from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics

monitor = AppleEnergyMonitor()

# This is where we want to start measuring energy from.
monitor.begin_window("example")

# Do some work...
x = 0
for i in range(10000):
    for j in range(1000):
        x += i + j

# End the measurement window and get results.
results: AppleEnergyMetrics = monitor.end_window("example")

# CPU related metrics
print("CPU Total", results.cpu_total_mj)
print("E Cores", results.efficiency_cores_mj)
print("P Cores", results.performance_cores_mj)
print("E Core Manager", results.efficiency_core_manager_mj)
print("P Core Manager", results.performance_core_manager_mj)

# DRAM
print("DRAM", results.dram_mj)

# GPU related metrics
print("GPU Total", results.gpu_mj)
print("GPU SRAM", results.gpu_sram_mj)

# ANE
print("ANE", results.ane_mj)

# Or, you could just print the whole thing like this:
# print(results)
