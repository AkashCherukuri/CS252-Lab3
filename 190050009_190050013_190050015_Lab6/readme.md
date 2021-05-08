# CS252-Lab6

Akash Cherukuri - 190050009
Aman Yadav      - 190050013
Amit Rajaraman  - 190050015

# Compilation and running code

### Requirements
The code is entirely automated. The confidence interval graphs are created using `matplotlib`, which requires Python to be installed.

- Install Python if not present. The version used for running the code is `Python 3.6.9`.
- Install matplotlib by typing the following command in your terminal.

  `pip3 install matplotlib`
  The version used for running the code is `matplotlib==3.3.4`.

### Running Experiments
The file of size exactly 5MB called `send.txt` is created using the `yes` command as follows:

`yes TheQuickBrownFoxJumpsOverTheLazyDog | head -c 5000000 > send.txt`

To run the experiments, firstly navigate to the folder with the code. Then, type in the following command.

`sudo ./run.sh`
The result of each experiment is displayed in `output.txt`, and the terminal is notified when the experiment is done for both TCP-Reno and TCP-Cubic. It also checks if `recv.txt` matches with `send.txt` in every iteration. The `result.txt` currently in the submission folder is the `output.txt`'s content (copy pasted) which we have used to make graphs and the report.

After all the experiments are done, plots are created and saved in the same folder automatically.

### Running server and client independently
First compile the `server.c` an `receiver.c` using the following commands:

`gcc -o client client.c`

`gcc -o server server.c`

Now you can set the loss and delay in the loop back interface using the following command:

`sudo tc qdisc change dev lo root netem loss <loss in %>% delay <delay in ms>ms`

Now run the server and then the client both of which take the TCP variant as the command line argument using the following command:

`./server <tcp variant>`

`./client <tcp variant>`

`<tcp variant>` can be reno or cubic.

Both the server and client terminate automatically when the transmission is done and recv.txt file is generated (which the server received) and throughput value(in bits/sec) is printed on terminal.

Note that the send.txt file needs to be in the same directory as `client.c`.
