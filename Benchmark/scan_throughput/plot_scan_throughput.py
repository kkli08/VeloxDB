import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the CSV files for Linux and macOS (scan throughput data)
linux_data = pd.read_csv("linux_scan_throughput.csv")
macos_data = pd.read_csv("macos_scan_throughput.csv")

# Pivot the data to get MemtableSize as columns and DataSizeMB as index
linux_pivot = linux_data.pivot(index='DataSizeMB', columns='MemtableSizeMB', values='Throughput(MB/s)')
macos_pivot = macos_data.pivot(index='DataSizeMB', columns='MemtableSizeMB', values='Throughput(MB/s)')

# Define the color scheme for Linux and macOS
# Linux: Shades of purple, macOS: Shades of orange
linux_colors = ['#9370DB', '#7B68EE', '#4B0082']  # Medium Purple, Medium Slate Blue, Indigo
macos_colors = ['#FFB347', '#FF8C00', '#FF4500']  # Light Orange, Dark Orange, Orange Red

# Set up the plot
plt.figure(figsize=(10, 6))

# Plot Linux data
x_values = linux_pivot.index.to_numpy()
for i, col in enumerate(linux_pivot.columns):
    y_values = linux_pivot[col].to_numpy()
    plt.plot(x_values, y_values, color=linux_colors[i % len(linux_colors)], marker='o', linestyle='-',
             label=f'Linux {col} MB')

# Plot macOS data with solid lines
x_values = macos_pivot.index.to_numpy()
for i, col in enumerate(macos_pivot.columns):
    y_values = macos_pivot[col].to_numpy()
    plt.plot(x_values, y_values, color=macos_colors[i % len(macos_colors)], marker='^', linestyle='-',
             label=f'macOS {col} MB')

# Set the title and labels
plt.title('Scan Throughput for Different Memtable Sizes (Linux vs. macOS)')
plt.xlabel('Input Data Size (MB)')
plt.ylabel('Throughput (MB/s)')

# Set custom x-axis values and labels
x_ticks = [2 ** i for i in range(5, 13)]  # Use 2, 4, 8, 16, 32, ..., 512
ax = plt.gca()  # Get current axis
ax.set_xscale('log', base=2)  # Set log scale for x-axis to maintain equal spacing

# Set tick positions and labels manually
ax.set_xticks(x_ticks)  # Set the position of the x ticks to match the values
ax.set_xticklabels([str(x) for x in x_ticks])  # Set the labels to display the actual numbers

# Customize the legend
plt.legend(title='Memtable Size (MB)', loc='upper right', fontsize=9)

# Remove gridlines (disable gridlines)
plt.grid(False)  # Completely remove all gridlines

# Remove border lines (spines) on the top and right
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)

# Optional: Remove left and bottom spines if desired
# ax.spines['left'].set_visible(False)
# ax.spines['bottom'].set_visible(False)

# Show the plot
plt.show()
