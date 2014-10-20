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

#define MAXBUFLEN 4
#define PACKET_SIZE 50
#define RECV_TIMEOUT 2


typedef struct packet_header {
	int packet_number;
} packet_header_t;

void reliablyReceive(unsigned short int myUDPport, char* destinationFile);

int main(int argc, char** argv)
{
	unsigned short int udpPort;
	
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}
	
	udpPort = (unsigned short int)atoi(argv[1]);
	char * filename = malloc(100);
	sprintf(filename, argv[2]);
	
	reliablyReceive(udpPort, filename);
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile)
{
    // expect handshake
    /*
** listener.c -- a datagram sockets "server" demo
*/


    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    // char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    char* port = malloc(sizeof(char)*6);
    sprintf(port, "%d", myUDPport);

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    free(port);
    // printf("i'm a penguin");
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");


    // handle handshake

    // addr_len = sizeof their_addr;
    // int *handshake = (int*)malloc(sizeof(int));
	// *handshake = 0;

 //    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN , 0,
 //        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
 //        perror("recvfrom handshake");
 //        exit(1);
	// }

	// *handshake = 0;
	// if ((numbytes = sendto(sockfd, 0, sizeof(int), 0,
	// 	p->ai_addr, p->ai_addrlen)) == -1) {
	// 	perror("talker: sendto");
	// 	exit(1);
	// }


    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
    int expected_packet_number = 0; // takes place of handshake
    // packet_header_t* ack_header = malloc(sizeof(packet_header_t));
    // ack_header->packet_number = 0;
	
	// handle getting packets
	int done = 0;
    int recv_bytes = 0;
	char packets[PACKET_SIZE];
    packet_header_t ack_buf;
	FILE* f = fopen(destinationFile,"wb+");
	while (!done) {
		if ((numbytes = recvfrom(sockfd, &packets, PACKET_SIZE , 0,
	        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	        perror("recvfrom handshake");
	        exit(1);
		}
        //printf("packets data: %s\n",packets);
        printf("numbytes: %d\n", numbytes);
        recv_bytes = numbytes;
        if (numbytes == 0) {
            done = 1;
            continue;
        }
		ack_buf.packet_number = packets[numbytes-sizeof(packet_header_t)];
        //printf("ack_buf.packet number = %d\n",ack_buf.packet_number);
        //printf("expected num = %d\n", expected_packet_number); 
		if (ack_buf.packet_number == expected_packet_number) {
			//send ack to sender
			printf("Got packet %d\n", expected_packet_number);
			if ((numbytes = sendto(sockfd, &ack_buf, sizeof(packet_header_t), 0,
				(struct sockaddr *)&their_addr, addr_len)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			// printf("ack buf nigga %d\n", ack_buf->packet_number); 
			fwrite(packets,1,recv_bytes-sizeof(packet_header_t),f);
			//increment expect pack num
			 expected_packet_number++; 
		}
		else {

			ack_buf.packet_number = expected_packet_number-1; // send dupACK
			printf("Sending dupACK %d\n", expected_packet_number-1);
			if ((numbytes = sendto(sockfd, &ack_buf, sizeof(packet_header_t), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			exit(1); 
		}
	}
    fclose(f);


    // printf("listener: got packet from %s\n",
    //     inet_ntop(their_addr.ss_family,
    //     get_in_addr((struct sockaddr *)&their_addr),
    //     s, sizeof s));
    // printf("listener: packet is %d bytes long\n", numbytes);
    // buf[numbytes] = '\0';
    // printf("listener: packet contains \"%s\"\n", buf);

    close(sockfd);

    // return 0;
	
}
