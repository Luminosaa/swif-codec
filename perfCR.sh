#!/bin/bash

# filepath: /home/thomas/Desktop/UGA/M1/Internship/swif-codec/test_performance.sh
CR=0.25
DT=15
# Define the range of x (loss rate) and y (window size)
loss_rates=$(seq 0 0.02 1) # x values: from 1 to 0 with a step of 0.05
window_sizes=(2 5 8 10 15 20) # y values: different window sizes

# Output file for results
output_file="results.csv"
echo "loss_rate,window_size,source_symbols_available" > $output_file

# Function to extract the number of source symbols available from the client output
extract_source_symbols() {
    grep "source symbols available" | awk '{print $1}'
}

# Loop through each window size (y)
for window_size in "${window_sizes[@]}"; do
    echo "Testing for window size: $window_size"
    
    # Loop through each loss rate (x)
    for loss_rate in $loss_rates; do
        echo "Testing for loss rate: $loss_rate"

        # Start the client in the background and redirect its output to a temporary file
        client_output="client_output.txt"
        ./applis/simple_client_server/simple_client > $client_output &
        client_pid=$!

        # Wait a moment to ensure the client is ready
        sleep 0.5

        # Start the server with the current loss rate and window size
        ./applis/simple_client_server/simple_server $loss_rate $window_size $CR $DT > /dev/null 2> /dev/null &

        # Wait for the client to finish
        wait $client_pid

        # Extract the number of source symbols available from the client output
        source_symbols=$(cat $client_output | extract_source_symbols)

        # Save the results to the output file
        echo "$loss_rate,$window_size,$source_symbols" >> $output_file

        # Clean up temporary files
        rm -f $client_output
    done
done

python3 plot_resultsCR.py $CR
echo "Performance test completed. Results saved to $output_file and plot saved to performance_plot.png."