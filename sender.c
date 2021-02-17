/*
References:
https://www.codeproject.com/Questions/557011/UdjustplusUDPplussocketplusdataplusreceivepluswait
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define MAXBUFLEN 100


// function to count number of digits in a number

int count_digits(int x) {
	int count =0;
	while(x!=0) {
		x = x/10;
		count++;
	}
	return count;
}



int main(int argc, char *argv[]) {

	//handling the command line arguments

	if(argc!=5) {
		printf("Usage: sender.c <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>\n");
		exit(1);
	}

	char* SenderPort = argv[1];
	char* ReceiverPort = argv[2];
	int RetranmissionTimer = atoi(argv[3]);
	int NoOfPacketsToBeSent = atoi(argv[4]);

	// creating socket for sending message to the receiver
	int sockfd_sender;
	struct addrinfo hints_sender, *servinfo_sender, *q;
	int sd;

	// initialize the sender socket
	memset(&hints_sender, 0, sizeof(hints_sender));
	hints_sender.ai_family = AF_UNSPEC;
	hints_sender.ai_socktype = SOCK_DGRAM;

	if((sd = getaddrinfo("localhost", ReceiverPort, &hints_sender, &servinfo_sender)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sd));
		return 1;
	}

	for(q = servinfo_sender; q != NULL; q = q->ai_next) {
		if((sockfd_sender = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
			perror("sender: socket");
			continue;
		}
		break;
	}

	if(q == NULL) {
		fprintf(stderr, "sender1: failed to bind socket\n");
    exit(1);
	}

	// creating sockets for receiving messages from the receiver
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;


	//create a fd_set consisting of sockfd
	fd_set master;
	fd_set read_fds;
	

	// Time data structure
	struct timeval tv;
	fd_set readfds;


	// initialize the receiver sockets
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, SenderPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("sender: socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("sender: bind");
			continue;
		}
		break;
	}

	if(p == NULL) {
		fprintf(stderr, "sender: failed to bind socket\n");
    return 2;
	}
	freeaddrinfo(servinfo); 

	FD_ZERO(&read_fds);
	FD_ZERO(&master);
	FD_SET(sockfd, &master);
	tv.tv_sec = RetranmissionTimer;
	tv.tv_usec = 0;
	read_fds = master;
	// initialize the packet sequence
	int x = 1;

	struct sockaddr_storage sender_address;
	socklen_t addr_len = sizeof(sender_address);
	int numbytes;
	char buf[MAXBUFLEN];


	while(x <= NoOfPacketsToBeSent) {

		// send the packet with packet sequence x
		int sent_bytes;
		char m[20];
		sprintf(m, "Packet:%d", x);
		m[7+count_digits(x)] = '\0';
		printf("%s\n", m);
		printf("%lu\n", strlen(m));
		if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
			perror("sender: sendto");
			exit(1);
		}

		printf("Packet with sequence number %d sent\n", x);


		// wait for the acknowledgement
		struct timeval tv_temp = tv;
		struct timeval tv_temp2;

		clock_t t;
		int ret;
		double time_taken;
		char sequence[100];
		int seq;
		label: 
   	t = clock();
   	tv_temp2 = tv_temp;

		ret = select(sockfd+1, &readfds, NULL, NULL, &tv_temp);

		tv_temp = tv_temp2;

		t = clock() - t;
   	time_taken = ((double)t)/CLOCKS_PER_SEC;

		if( ret < 0 ) {
			perror("select");
			exit(1);
		}
		else if ( ret == 0 ) {
			//timeout
			printf("Restransmission timer of %d packet expired", x);
		}
		else if ( FD_ISSET( sockfd, &read_fds)) {
			
			if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&sender_address, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
			}

			// parse the received message and check the sequence number
			strncpy(sequence, buf+15, numbytes-15);
			sequence[numbytes-15] = '\0';
			seq = atoi(sequence);

			// check if this is the acknowledgement of the recently sent packet
			if(seq == x+1) {
				x++;
				printf("ACK of %d packet received", x);
			}
			else {
				tv_temp.tv_sec = tv_temp.tv_sec- time_taken;
				goto label;
				printf("ACK of %d received and ignored", seq);
			}
		}
	}
	// close the sockets
	freeaddrinfo(servinfo_sender);
	close(sockfd);
	close(sockfd_sender);
} 
