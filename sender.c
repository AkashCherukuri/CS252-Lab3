/*
sender.c 
*/

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

	memset(&hints_sender, 0, sizeof(hints_sender));
	hints_sender.ai_family = AF_UNSPEC;
	hints_sender.ai_socktype = SOCK_DGRAM;
	hints_sender.ai_flags = AI_PASSIVE;


	if((sd = getaddrinfo(NULL, ReceiverPort, &hints_sender, &servinfo_sender)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sd));
		return 1;
	}

	for(q = servinfo_sender; q != NULL; q = q->ai_next) {
		if((sockfd_sender = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
			perror("sender: socket");
			continue;
		}

		if(bind(sockfd_sender, q->ai_addr, q->ai_addrlen) == -1) {
			close(sockfd_sender);
			perror("sender: bind");
			continue;
		}
		break;
	}

	if(q == NULL) {
		fprintf(stderr, "sender: failed to bind socket\n");
    return 2;
	}

	// creating sockets for receiving messages from the receiver

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

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

	//create a fd_set consisting of sockfd
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&master);
	FD_SET(sockfd, &master);

	// Time data structure
	struct timeval tv;
	fd_set readfds;
	tv.tv_sec = RetranmissionTimer;
	tv.tv_usec = 0;

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
		if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
			perror("sender: sendto");
				exit(1);
		}

		// wait for the acknowledgement
		read_fds = master;
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
		}
		else if ( FD_ISSET( sockfd, &read_fds)) {
			
			if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&sender_address, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
			}

			// parse the received message and check the sequence number
			strncpy(sequence, buf+15, numbytes);
			sequence[numbytes] = '\0';
			seq = atoi(sequence);

			// check if this is the acknowledgement of the recently sent packet
			if(seq == x+1) {
				x++;
			}
			else {
				tv_temp.tv_sec = tv_temp.tv_sec- time_taken;
				goto label;
			}
		}

	}


	// close the sender sockets
	freeaddrinfo(servinfo_sender);
	close(sockfd_sender);

	// close the receiver sockets
	close(sockfd);
} 