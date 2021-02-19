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

	FILE *fp;
	fp = fopen("sender.txt", "w");

	if ( fp == NULL ) { 
		printf( "sender.txt file failed to open." ) ; 
	}
	char data[1000];
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

	// initialize the packet sequence
	int x = 1;

	struct sockaddr_storage sender_address;
	socklen_t addr_len = sizeof(sender_address);
	ssize_t numbytes;
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

		sprintf(data, "Packet with sequence number %d sent\n\n", x);
		printf("%s", data);
		fputs(data, fp);
		// wait for the acknowledgement
		// set timeout on socket
		struct timeval tv;
		tv.tv_sec = RetranmissionTimer;
		tv.tv_usec = 0;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
		time_t begin;
		double time_taken;
		char sequence[100];
		int seq;
		double total_time_taken = 0;
		label: 
   	// printf("hi\n");
		time(&begin);		
   	numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&sender_address, &addr_len);
		// printf("%zd\n", numbytes);;

		// printf("bye\n");
		time_t end;
		time(&end);
		time_taken = difftime(end,begin);
   	total_time_taken += time_taken;
   	// printf("total_timetaken:%f\n", total_time_taken);
   	if(numbytes < 0) {
			continue;
		}
		else {
			// parse the received message and check the sequence number
			strncpy(sequence, buf+15, numbytes-15);
			sequence[numbytes-15] = '\0';
			seq = atoi(sequence);

			// check if this is the acknowledgement of the recently sent packet
			if(seq == x+1) {
				x++;
				sprintf(data, "ACK of %d packet received\n", x);
				printf("%s", data);
				fputs(data, fp);
			}
			else {
				sprintf(data, "ACK of %d received and ignored\n", seq);
				printf("%s", data);
				fputs(data, fp);
				struct timeval tv_temp;
				tv_temp.tv_sec = RetranmissionTimer-total_time_taken;
				tv_temp.tv_usec = 0;
				setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_temp, sizeof tv_temp);
				goto label;
			}
		}
	}

	int sent_bytes;
	char m[20];
	sprintf(m, "Packet:%d", 0);
	m[7+count_digits(1)] = '\0';
	if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
		perror("sender: sendto");
		exit(1);
	}


	// close the sockets
	freeaddrinfo(servinfo_sender);
	close(sockfd);
	close(sockfd_sender);
	fclose(fp);
} 
