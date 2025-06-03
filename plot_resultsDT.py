import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np
import argparse

parser = argparse.ArgumentParser(description='Plot source symbols vs loss rate for different DT values')
parser.add_argument('coding_rate', type=float, help='Coding rate (e.g., 0.25)')
args = parser.parse_args()
coding_rate = args.coding_rate

# Load the CSV data
data = pd.read_csv('results_DT.csv')

plt.rcParams.update({
    'font.size': 14,
    'axes.titlesize': 16,
    'axes.labelsize': 16,
    'xtick.labelsize': 14,
    'ytick.labelsize': 14,
    'legend.fontsize': 14
})

fig, ax = plt.subplots(figsize=(8, 6))

# Plot one line per DT value
for DT, group in data.groupby('DT'):
    ax.plot(group['loss_rate'], group['source_symbols_available'],
            label=f'DT {DT}', marker='o')

# Compute and plot asymptotic curve
loss_rates = np.linspace(0, 1, 100)
k = int(1 / coding_rate)
asymptotic_recovery = (1 - loss_rates**k) * data['source_symbols_available'].max()
ax.plot(loss_rates, asymptotic_recovery, 'k--', label='Duplicate Sending (asymptotic)')
ax.axvline(x=0.66, color='red', linestyle='--', linewidth=2)

ax.set_xlabel('Loss Rate')
ax.set_ylabel('Source Symbols Available')
ax.set_title(f'Source Symbols vs Loss Rate (Coding Rate = {coding_rate:.2f})')
ax.grid(True)
ax.legend()
plt.tight_layout()

plt.savefig('source_symbols_vs_loss_rate_DT.png', dpi=300)
plt.show()
