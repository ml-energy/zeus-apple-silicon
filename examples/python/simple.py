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

# CPU related metrics (all in mJ)
print("CPU Total (mJ):", results.cpu_total_mj)
print("Efficiency Cores (mJ):", results.efficiency_cores_mj)
print("Performance Cores (mJ):", results.performance_cores_mj)
print("Efficiency Cluster (mJ):", results.efficiency_cluster_mj)
print("Performance Cluster (mJ):", results.performance_cluster_mj)
print("Efficiency Core Manager (mJ):", results.efficiency_core_manager_mj)
print("Performance Core Manager (mJ):", results.performance_core_manager_mj)

# DRAM
print("DRAM (mJ):", results.dram_mj)

# GPU related metrics
print("GPU Total (mJ):", results.gpu_mj)
print("GPU SRAM (mJ):", results.gpu_sram_mj)

# ANE
print("ANE (mJ):", results.ane_mj)

# Or, you could just print the whole thing like this:
# print(results)
