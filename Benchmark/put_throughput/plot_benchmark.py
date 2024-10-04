import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
data = pd.read_csv("put_throughput.csv")

# Pivot the data to get MemtableSize as columns and DataSizeMB as index
pivot_data = data.pivot(index='DataSizeMB', columns='MemtableSize', values='Throughput(MB/s)')

# Plot the graph
pivot_data.plot(kind='line', marker='o')

# Set the title and labels
plt.title('Put Throughput for Different Memtable Sizes')
plt.xlabel('Input Data Size (MB)')
plt.ylabel('Throughput (MB/s)')
plt.xscale('log', base=2)  # Log scale for X-axis (data size)
plt.grid(True)
plt.legend(title='Memtable Size (MB)')
plt.show()
