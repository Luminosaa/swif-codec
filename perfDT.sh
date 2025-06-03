#!/bin/bash

CR=0.25
WINDOW_SIZE=20
# Define the range of x (loss rate) and y (DT)
loss_rates=$(seq 0 0.02 1) # x values: from 0 to 1 with a step of 0.02
DT_values=$(seq 0 3 15)    # y values: DT from 0 to 15

# Output file for results
output_file="results_DT.csv"
echo "loss_rate,DT,source_symbols_available" > $output_file

# Function to extract the number of source symbols available from the client output
extract_source_symbols() {
    grep "source symbols available" | awk '{print $1}'
}

# Loop through each DT value
for DT in $DT_values; do
    echo "Testing for DT: $DT"
    
    # Loop through each loss rate
    for loss_rate in $loss_rates; do
        echo "Testing for loss rate: $loss_rate"

        # Start the client in the background and redirect its output to a temporary file
        client_output="client_output.txt"
        ./applis/simple_client_server/simple_client > $client_output &
        client_pid=$!

        # Wait a moment to ensure the client is ready
        sleep 0.5

        # Start the server with the current loss rate, fixed window size, CR, and current DT
        ./applis/simple_client_server/simple_server $loss_rate $WINDOW_SIZE $CR $DT > /dev/null 2> /dev/null &

        # Wait for the client to finish
        wait $client_pid

        # Extract the number of source symbols available from the client output
        source_symbols=$(cat $client_output | extract_source_symbols)

        # Save the results to the output file
        echo "$loss_rate,$DT,$source_symbols" >> $output_file

        # Clean up temporary files
        rm -f $client_output
    done
done

python3 plot_resultsDT.py $CR
echo "Performance test completed. Results saved to $output_file and plot saved to performance_plot.png."
