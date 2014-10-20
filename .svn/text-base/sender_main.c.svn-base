#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>

typedef struct packet_header {
	int packet_number;
} packet_header_t;

/*
typedef struct timeout_list {
    int timestamp; // int value of time()
    int packet_number;
    struct timeout_list * next;
} timeout_list_node_t;
*/

#define PACKET_SIZE (50 - sizeof(packet_header_t))
#define WINDOW_SIZE 10
#define RECV_TIMEOUT 2 // seconds

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer);

int main(int argc, char** argv)
{
	unsigned short int udpPort;
	unsigned long long int numbytes;
	
	if(argc != 5)
	{
		fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
		exit(1);
	}
	udpPort = (unsigned short int)atoi(argv[2]);
	numbytes = atoll(argv[4]);
	// printf("I'm a penguin. in main.\n");
	char * hostname = malloc(100);
	sprintf(hostname, argv[1]);
	char * filename = malloc(100);
	sprintf(filename, argv[3]);
	reliablyTransfer(hostname, udpPort, filename, numbytes);
} 

void reliablyTransfer(char* hostname, 
					  unsigned short int hostUDPport, 
					  char* filename, 
					  unsigned long long int bytesToTransfer)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr; 
	socklen_t addr_len;
	addr_len = sizeof(their_addr);
	int sending = 1; 


	// if (argc != 3) {
	// 	fprintf(stderr,"usage: talker hostname message\n");
	// 	exit(1);
	// }

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	// p/rintf("%s and %s\n", hostname, hostUDPport);

	char* portno = malloc(sizeof(char)*6);
	sprintf(portno, "%d", hostUDPport);
	if ((rv = getaddrinfo(hostname, portno, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	free(portno);
	// printf("I'm a penguin. in reliable 2.\n");
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}


		break;
	}

	// set socket timeout for recv calls
	struct timeval tv;
	tv.tv_sec = RECV_TIMEOUT;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	// printf("I'm a penguin. in reliable 3.\n");
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1);
	}

	int *handshake = (int*)malloc(sizeof(int));
	*handshake = 0;
	char * buf = (char*)malloc(PACKET_SIZE + sizeof(packet_header_t));
	FILE* f = fopen(filename,"r");
	int num_packets = (bytesToTransfer/PACKET_SIZE)+1;
    int last_packet_size = bytesToTransfer % PACKET_SIZE;

	// initiate handshake
	// int fail = 1; 
	// while (fail == 1) {
	// 	if ((numbytes = sendto(sockfd, handshake, sizeof(int), 0,
	// 			 p->ai_addr, p->ai_addrlen)) == -1) {
	// 		perror("sendto handshake");
	// 		exit(1);
	// 	}

	// 	// wait for handshake response 
	// 	if ((numbytes = recvfrom(sockfd, buf, sizeof(int) , 0,
	//         (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	//         perror("recvfrom handshake");
	//         exit(1);
	//     }
	
	//     if (numbytes == sizeof(int)) {
	//     	fail = 0; // successful handshake 
	//     }
	// }

	// read file into one big chunk
	char *packet_chunk = malloc(num_packets * PACKET_SIZE);
	fread(packet_chunk, PACKET_SIZE, num_packets, f);

	// array of chunks including packet header
	int i,j;
	char* packets[num_packets];
	packet_header_t pheader;
	//printf("num_packets %d\n", num_packets); 
	for (i = 0; i < num_packets; i++) {
		pheader.packet_number = i;
        if (i == num_packets-1)
        {   
            printf("packet number for  last nigga %d \n", pheader.packet_number); 
            packets[i] = malloc(last_packet_size + sizeof(packet_header_t));
            memcpy(packets[i],packet_chunk + i*PACKET_SIZE, last_packet_size);
            memcpy(packets[i] + last_packet_size, &pheader, sizeof(packet_header_t)); 
        }
        else
        {  
		    packets[i] = malloc(PACKET_SIZE + sizeof(packet_header_t));
            memcpy(packets[i], packet_chunk + i*PACKET_SIZE, PACKET_SIZE);
		    memcpy(packets[i] + PACKET_SIZE, &pheader, sizeof(packet_header_t));
        }
		
	}
	free(packet_chunk);

	// start sending packets
	int next_packet_number = 0;
	int expected_ack = 0;
	// int curr_window = WINDOW_SIZE;
	packet_header_t* ack_buf = malloc(sizeof(packet_header_t));

	// tcp logic
	while (sending == 1) {
		// send as many packets as we can
		while (next_packet_number < expected_ack + WINDOW_SIZE && (next_packet_number < num_packets)) {
			// printf("%s\n", packets[next_packet_number]);
			// printf("")
            if (next_packet_number == num_packets-1) {
                if ((numbytes = sendto(sockfd, packets[next_packet_number], last_packet_size + sizeof(packet_header_t), 0,
				    p->ai_addr, p->ai_addrlen)) == -1) {
				    perror("talker: sendto");
				    exit(1);
			    }
            }
			else if ((numbytes = sendto(sockfd, packets[next_packet_number], PACKET_SIZE + sizeof(packet_header_t), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			printf("Sent packet %d\n", next_packet_number);
           	next_packet_number++;
		}
		// printf("iamhere \n"); 
 		// wait for ack recieved
 		if ((numbytes = recvfrom(sockfd, ack_buf, sizeof(packet_header_t) , 0,
	        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	        perror("recvfrom handshake");
	        exit(1);
	    }
        
        // handle timeout
	    if (numbytes == 0) {
            // resend window from expected_ack to next_packet_number-1
            printf("Something timed out on packet %d\n", next_packet_number);
            for (j = expected_ack; j < next_packet_number; j++) {
                if ((numbytes = sendto(sockfd, packets[j], PACKET_SIZE + sizeof(packet_header_t), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
				}
            }
	    }
	    else
	    {
		    // iterate over received list of acks
	    	// packet_header_t temp = *ack_buf;
	    	if (ack_buf->packet_number == expected_ack) {
	    		printf("Got ack %d\n", expected_ack);
	    		expected_ack++;
	    	}
	    	else { // handle dupAck
	    		// resend from dupACK number + 1
	    		printf("Sending stuff for dupACK %d\n", expected_ack);
	    		for (i = ack_buf->packet_number + 1; i < next_packet_number; i++) {
	    			if ((numbytes = sendto(sockfd, packets[i], PACKET_SIZE + sizeof(packet_header_t), 0,
						p->ai_addr, p->ai_addrlen)) == -1) {
						perror("talker: sendto");
						exit(1);
					}
	    		}
	    	}

	        // check if we're done
	        if (num_packets == expected_ack) {
                printf("we out\n");
	            sending = 0;
	        }
	    }
	}

	freeaddrinfo(servinfo);

	// printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);
}
