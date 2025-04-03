from zeus_apple_silicon import AppleEnergyMonitor, AppleEnergyMetrics

mon = AppleEnergyMonitor()

res = mon.get_cumulative_energy()
print(res)
