# This code makes graphs after the main part of run.sh is finished

import matplotlib.pyplot as plt
from math import sqrt

# Store the losses and delays to be able to use later
losses = [0.1, 0.5, 1]
delays = [10, 50, 100]

# Have the factor ready, multiply with std_deviation to get the confidence interval
conf_fac = 1.645/sqrt(20)

reno_mean = []
reno_std = []
cubic_mean= []
cubic_std = []

# Read output.txt and store in t_data
with open("output.txt") as out_file:
	t_data = out_file.readlines()

# Store the data from output.txt into the corresponding array
# reno_mean stores the mean values of throughputs, reno_std stores the deviations of the throughputs
# similarly for cubic
for i in range(3):
	reno_mean.append([])
	reno_std.append([])
	cubic_mean.append([])
	cubic_std.append([])
	for j in range(3):
		#reno_mean[i][j] stores the mean value for loss corresponding to i and delay corresponding to j
		#0->10ms and 0.1%, 1->50ms and 0.5%, 2->100ms and 1%
		reno_mean[i].append(float(t_data[24+58*(j+3*i)][6:-1]))
		reno_std[i].append(float(t_data[25+58*(j+3*i)][20:-1]))

		cubic_mean[i].append(float(t_data[53+58*(j+3*i)][6:-1]))
		cubic_std[i].append(float(t_data[54+58*(j+3*i)][20:-1]))

# Throughput vs delay plots using above information
for i in range(3):
	loss = losses[i]
	fig = plt.figure()

	reno_up = []
	reno_bot= []

	cubic_up = []
	cubic_bot= []

	for j in range(3):
		reno_up.append(reno_mean[i][j]+conf_fac*reno_std[i][j])
		reno_bot.append(reno_mean[i][j]-conf_fac*reno_std[i][j])
		cubic_up.append(cubic_mean[i][j]+conf_fac*cubic_std[i][j])
		cubic_bot.append(cubic_mean[i][j]-conf_fac*cubic_std[i][j])

	plt.title(label='loss = '+str(loss)+'%')
	plt.xlabel('delay (in ms)')
	plt.ylabel('throughput (in bits/s)')
	plt.grid()

	plt.plot(delays, reno_mean[i], color="red")
	plt.fill_between(delays, reno_bot, reno_up, color='red', alpha=0.1)
	plt.plot(delays, cubic_mean[i], color="blue")
	plt.fill_between(delays, cubic_bot, cubic_up, color='blue', alpha=0.1)
	plt.legend(["Reno", "Cubic"], loc ='upper left')

	plt.savefig("Loss_"+str(loss)+".png")

# Throughput vs Loss plots using above information
for j in range(3):
	delay = delays[j]
	fig = plt.figure()

	reno_up = []
	reno_bot= []

	cubic_up = []
	cubic_bot= []

	for i in range(3):
		reno_up.append(reno_mean[i][j]+conf_fac*reno_std[i][j])
		reno_bot.append(reno_mean[i][j]-conf_fac*reno_std[i][j])
		cubic_up.append(cubic_mean[i][j]+conf_fac*cubic_std[i][j])
		cubic_bot.append(cubic_mean[i][j]-conf_fac*cubic_std[i][j])

	plt.title(label='delay = '+str(delay)+'ms')
	plt.xlabel('loss percent')
	plt.ylabel('throughput (in bits/s)')
	plt.grid()

	r_mean = []
	c_mean = []
	for i in range(3):
		r_mean.append(reno_mean[i][j])
		c_mean.append(cubic_mean[i][j])

	plt.plot(losses, r_mean, color="red")
	plt.fill_between(losses, reno_bot, reno_up, color='red', alpha=0.1)
	plt.plot(losses, c_mean, color="blue")
	plt.fill_between(losses, cubic_bot, cubic_up, color='blue', alpha=0.1)
	plt.legend(["Reno", "Cubic"], loc ='upper left')

	plt.savefig("Delay_"+str(delay)+".png")