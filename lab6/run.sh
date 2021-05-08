#!/bin/bash


# compile the server and client
gcc -o client client.c
gcc -o server server.c

# create the output file
touch output.txt
cat /dev/null > output.txt

#create arrays for each of the 3x3x2 experiments
losses=('0.1%' '0.5%' '1%')
delays=('10ms' '50ms' '100ms')
tcps=('reno' 'cubic')

for loss in "${losses[@]}"; do
	for delay in "${delays[@]}"; do
		for tcp in "${tcps[@]}"; do
			# setting the loss and delay

			# TCP cubic
			echo "Loss = $loss, delay = $delay, tcp $tcp
Throughputs for 20 runs:
" | cat >> output.txt

			# perform the experiment 20 times
			sum="0"
			sum_square="0"
			for i in {0..19}
			do
				sudo tc qdisc change dev lo root netem loss $loss delay $delay
				
				sleep 1.5
				./server $tcp &
				num=$(./client $tcp | tail -n 1)
				wait

				# check if send.txt and recv.txt are the same
				STATUS="$(cmp --silent send.txt recv.txt; echo $?)"
				
				#Declare that the code is buggy if they're not the same
				if [[ $STATUS -ne 0 ]]; then
					echo "recv.txt is different from send.txt .The code is buggy :("
				   exit 1
				fi

				echo "$num" | cat >> output.txt
				
				#Update sum and sum_square
				sum=$(awk '{print $1 + $2}' <<<"${sum} ${num}")
				sum_square=$(awk '{print $1 + $2^2}' <<<"${sum_square} ${num}")
			done

			#Calculate the mean and standard deviation
			avg=$(awk '{print $1/20}' <<<"${sum}")
			std=$(awk '{print sqrt($1/20 - $2^2)}' <<<"${sum_square} ${avg}")

			#Store the values in a file called "output.txt"
			echo "
Mean: $avg" | cat >> output.txt
			echo "Standard Deviation: $std

--------------------------------------------------------------------
" | cat >> output.txt
		done
		#Show terminal that both Cubic and Reno are finished for the given case
		echo "Finished case: loss=$loss, delay=$delay"
	done
done

#make the 6 plots automatically once the code is done running 
python3 graph.py