import matplotlib.pyplot as plt

# Example data (replace with your results)
cache_sizes = range(1, 21)
fifo_faults = [100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1]
lru_faults = [95, 85, 75, 65, 55, 45, 35, 25, 15, 10, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1]
rand_faults = [98, 88, 78, 68, 58, 48, 38, 28, 18, 12, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2]
opt_faults = [90, 80, 70, 60, 50, 40, 30, 20, 10, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1]

# Plot
plt.plot(cache_sizes, fifo_faults, label='FIFO')
plt.plot(cache_sizes, lru_faults, label='LRU')
plt.plot(cache_sizes, rand_faults, label='RAND')
plt.plot(cache_sizes, opt_faults, label='OPT')

# Labels and title
plt.xlabel('Cache Size (Number of Pages)')
plt.ylabel('Page Faults (log scale)')
plt.yscale('log')
plt.title('Page Faults vs Cache Size for Different Policies')
plt.legend()

# Save the plot
plt.savefig('simresult.png')
plt.show()