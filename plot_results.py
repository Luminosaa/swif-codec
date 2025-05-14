import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Load the results.csv file
data_file = "results.csv"
data = pd.read_csv(data_file)

# Create a plot for each window size
window_sizes = data["window_size"].unique()
plt.figure(figsize=(10, 6))

for window_size in window_sizes:
    subset = data[data["window_size"] == window_size]
    plt.plot(
        subset["loss_rate"] * 100,  # Convert loss rate to percentage
        subset["source_symbols_available"],
        marker="o",
        label=f"Window Size = {window_size}",
    )

# Add the asymptotic curve
loss_rates = np.linspace(0, 1, 100)  # Generate loss rates from 0 to 1
asymptotic_values = 1000 * (1 - loss_rates ** 3.333)
plt.plot(
    loss_rates * 100,  # Convert loss rates to percentage
    asymptotic_values,
    linestyle="--",
    color="black",
    label="Duplicate sending (asymptotic)",
)

# Configure the plot
plt.title("Performance of Implementation with a Coding Rate of 0.3")
plt.xlabel("Loss Rate (%)")
plt.ylabel("Number of Source Symbols Available")
plt.grid(True)
plt.legend()
plt.xticks(np.arange(0, 101, 5))
plt.tight_layout()

# Save the plot as a PNG file
output_plot = "performance_plot.png"
plt.savefig(output_plot)
print(f"Plot saved to {output_plot}")

# Show the plot
plt.show()