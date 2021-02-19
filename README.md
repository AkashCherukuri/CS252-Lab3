# CS 252 Lab 3
<!-- ======================================================================================================
                                    CS252-Lab3-Assignment-Readme
======================================================================================================
 -->
<!-- Teammates
 -->
Akash Cherukuri - 190050009    
Aman Yadav - 190050013    
Amit Rajaraman - 190050015    

## Description of Code

<!-- ======================================================================================================
										Description of Code
======================================================================================================
 -->

### `sender.c`

This code is used for sending the packet and waiting for the ACK packet.

Implementation has been done using two sockets - 
- `sockfd_sender` is used to send the sequence packet
- `sockfd` is used to receive the ACK frame

The code prints the packets sent and received to the terminal and the corresponding text file.

If the acknowledgment is not received within the Retransmission Timer, then the packet is 
resent after resetting the timer.

For setting the timeout on the `recvfrom()`, we have used `setsockopt()` function.

If the acknowledgement expected is not what we have received, in that case, we restart `recvfrom()`
with the timeout value equal to (transmission value - time taken).

End of transmission is signalled to the receiver by using a unique packet. (`seq` = 0)

<!-- ------------------------------------------------------------------------------------------------------ -->

### `receiver.c`

This code is used for receiving the packet and sending out the corresponding ACK packet.

Implementation is similar to Sender.c using two sockets.

Packet drop is simulated using `srand()` with the seed as `time(NULL)`.

If an unexpected packet is received, then acknowledgement with the correct expected packet number 
sequence is sent.

If the sequence number received is 0, then the operation is terminated.


## Compiling the Code

<!-- ======================================================================================================
										Compiling the Code
======================================================================================================
 -->
1. Simulate delay on local machine (optional)
         sudo tc qdisc add dev lo root netem delay<Delayinmiliseconds>

2. Compile and execute `receiver.c`
         gcc -o rec receiver.c    
         sudo ./rec <ReceiverPort> <SenderPort> <PacketDropProbability>

3. Compile and execute `sender.c`
         gcc -o send sender.c    
         sudo ./send <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>

Make sure to use ports > 1024 as the lower ones are in use by the system itself.

After the packets have been sent, the code terminates automatically, and the output is piped 
to the corresponding textfiles.

## References

<!-- ======================================================================================================
											References
======================================================================================================
 -->
https://www.codeproject.com/Questions/557011/UdjustplusUDPplussocketplusdataplusreceivepluswait
https://stackoverflow.com/questions/22661300/calculate-execution-time-when-sleep-is-used
https://stackoverflow.com/questions/6218399/how-to-generate-a-random-number-between-0-and-1