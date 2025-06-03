import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np

# -------- Configuration --------
import argparse

parser = argparse.ArgumentParser(description='Plot source symbols vs loss rate with coding rate')
parser.add_argument('coding_rate', type=float, help='Coding rate (e.g., 0.5 for 1:2 redundancy)')
args = parser.parse_args()
coding_rate = args.coding_rate

# Load the CSV data
data = pd.read_csv('results.csv')  # Replace with actual CSV filename

# Set up plot appearance
plt.rcParams.update({
    'font.size': 14,
    'axes.titlesize': 16,
    'axes.labelsize': 16,
    'xtick.labelsize': 14,
    'ytick.labelsize': 14,
    'legend.fontsize': 14
})

# Create the plot
fig, ax = plt.subplots(figsize=(8, 6))

# Plot one line per window_size
for window_size, group in data.groupby('window_size'):
    ax.plot(group['loss_rate'], group['source_symbols_available'],
            label=f'Window size {window_size}', marker='o')

# Compute and plot asymptotic curve
loss_rates = np.linspace(0, 1, 100)
k = int(1 / coding_rate)
asymptotic_recovery = (1 - loss_rates**k) * data['source_symbols_available'].max()
ax.plot(loss_rates, asymptotic_recovery, 'k--', label='Duplicate Sending (asymptotic)')
ax.axvline(x=0.66, color='red', linestyle='--', linewidth=2)
# Labels, title, and legend
ax.set_xlabel('Loss Rate')
ax.set_ylabel('Source Symbols Available')
ax.set_title(f'Source Symbols vs Loss Rate (Coding Rate = {coding_rate:.2f})')
ax.grid(True)
ax.legend()
plt.tight_layout()

# Save and show
plt.savefig('source_symbols_vs_loss_rate_with_asymptotic.png', dpi=300)
plt.show()
