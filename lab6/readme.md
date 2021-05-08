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
The result of each experiment is displayed in `output.txt`, and the terminal is notified when the experiment is done for both TCP-Reno and TCP-Cubic. It also checks if `recv.txt` matches with `send.txt` in every iteration.

After all the experiments are done, plots are created and saved in the same folder automatically.