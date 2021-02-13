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

int count_digits(int x) {
	int count =0;
	while(x!=0) {
		x = x/10;
		count++;
	}
	return count;
}

int main(int argc, char* argv[]) {

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
		strncpy(sequence, buf+7, numbytes);
		sequence[numbytes] = '\0';
		int seq = atoi(sequence);

		// create a socket to send ACK to the sender

		int sockfd_sender;
		struct addrinfo hints_sender, *servinfo_sender, *q;
		int sd;

		memset(&hints_sender, 0, sizeof(hints_sender));
		hints_sender.ai_family = AF_UNSPEC;
		hints_sender.ai_socktype = SOCK_DGRAM;
		hints_sender.ai_flags = AI_PASSIVE;


		if((sd = getaddrinfo(NULL, SenderPort, &hints_sender, &servinfo_sender)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		for(q = servinfo_sender; q != NULL; q = q->ai_next) {
			if((sockfd_sender = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
				perror("receiver: socket");
				continue;
			}

			if(bind(sockfd_sender, q->ai_addr, q->ai_addrlen) == -1) {
				close(sockfd_sender);
				perror("receiver: bind");
				continue;
			}
			break;
		}

		if(q == NULL) {
			fprintf(stderr, "receiver: failed to bind socket\n");
	    return 2;
		}

		// check if the sequence received doesn't match the sequece expected
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
			printf("Package with sequence number %d dropped\n", seq);
		}
		else {
			double random_value;
			srand(time(NULL));
			random_value = (double)rand()/RAND_MAX;

			if(random_value >= PacketDropProbability) {
				// Packet received successfully and send an acknowledgement with next sequence number
				int sent_bytes;
				char m[20];
				x++;
				sprintf(m, "Acknowledgment:%d\0", x);
				m[15+count_digits(x)] = '\0';
				if((sent_bytes = sendto(sockfd_sender, m, strlen(m), 0, q->ai_addr, q->ai_addrlen)) == -1) {
					perror("receiver: sendto");
						exit(1);
				}
			}
			else {
				// drop the packet
				printf("Package with sequence number %d dropped\n", seq);
			}
		}
		freeaddrinfo(servinfo_sender);
		close(sockfd_sender);
	}
	close(sockfd);
}

