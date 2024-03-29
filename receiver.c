#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
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

int main(int argc, char* argv[]) {

	FILE *fp;
	fp = fopen("receiver.txt", "w") ;

	if ( fp == NULL ) { 
		printf( "receiver.txt file failed to open." ) ; 
	}

	char data[1000];
	// handling the command line arguments
	if(argc!=4) {
		printf("Usage: receiver.c <ReceiverPort> <SenderPort> <PacketDropProbability>\n");
		exit(1);
	}

	char* ReceiverPort = argv[1];
	char* SenderPort = argv[2];
	double PacketDropProbability = atof(argv[3]);

	// setting up socket for receiving and sending messages
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	// initilize socket for receiving
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, ReceiverPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("receiver: socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("receiver: bind");
			continue;
		}
		break;
	}

	if(p == NULL) {
		fprintf(stderr, "receiver: failed to bind socket\n");
    return 2;
	}
	freeaddrinfo(servinfo);

	int sockfd_sender;
	struct addrinfo hints_sender, *servinfo_sender, *q;
	int sd;
	// create a socket to send ACK to the sender
	memset(&hints_sender, 0, sizeof(hints_sender));
	hints_sender.ai_family = AF_UNSPEC;
	hints_sender.ai_socktype = SOCK_DGRAM;

	if((sd = getaddrinfo("localhost", SenderPort, &hints_sender, &servinfo_sender)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(q = servinfo_sender; q != NULL; q = q->ai_next) {
		if((sockfd_sender = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
			perror("receiver: socket");
			continue;
		}
		break;
	}

	if(q == NULL) {
		fprintf(stderr, "receiver: failed to bind socket\n");
    return 2;
	}

	// initilialize the sequence number
	int x = 1;

	struct sockaddr_storage sender_address;
	socklen_t addr_len = sizeof(sender_address);
	int numbytes;
	char buf[MAXBUFLEN];

	while(1) {

		// wait until we receive a packet
		if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&sender_address, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		// parse the received message and check the sequence number
		char sequence[100];
		strncpy(sequence, buf+7, numbytes-7);
		sequence[numbytes-7] = '\0';
		// printf("%s\n", sequence);
		int seq = atoi(sequence);
		sprintf(data, "Packet with sequence number %d received\n", seq);
		if (seq!=0) printf("%s", data);
		fputs(data, fp);

		if(seq == 0) break;

		// check if the sequence received doesn't match the sequence expected
		if(seq != x) {
			// if they don't match then send the packet with expected sequence number
			int sent_bytes;
			char m[50];
			sprintf(m, "Acknowledgment:%d", x);
			m[15+count_digits(x)] = '\0';
			if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
				perror("receiver: sendto");
				exit(1);
			}
			sprintf(data, "Package with sequence number %d dropped (not equal to expected seq no. %d) and ACK for %d sent\n", seq, x, x);
			printf("%s", data);
			fputs(data, fp);
		}
		else {
			srand(time(NULL));
			double random_value;
			random_value = (double)rand()/(double)RAND_MAX;
			//printf("%f", random_value);
			if(random_value >= PacketDropProbability) {
				// Packet received successfully and send an acknowledgement with next sequence number
				int sent_bytes;
				char m[20];
				x++;
				sprintf(m, "Acknowledgment:%d", x);
				m[15+count_digits(x)] = '\0';
				if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
					perror("receiver: sendto");
					exit(1);
				}
				sprintf(data, "Packet with sequence number %d accepted and ACK for %d sent\n\n", seq, x);
				printf("%s", data);
				fputs(data, fp);
			}
			else {
				// drop the packet
				sprintf(data, "Packet with sequence number %d dropped to simulate packet loss\n\n", seq);
				printf("%s", data);
				fputs(data, fp);
			}
		}
	}
	close(sockfd);
	close(sockfd_sender);
	freeaddrinfo(servinfo_sender);
	fclose(fp);
}

