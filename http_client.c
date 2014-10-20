/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "80" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char * buf = malloc(MAXDATASIZE);
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    // copy arg string before tokenizing
    char * original_string = malloc(sizeof(char)*strlen(argv[1]));
    strcpy(original_string, argv[1]);

    // split string
    char * tok = NULL;
    char * path = NULL;
    tok = strtok(argv[1],"/");
    path = strtok(NULL, "/");
    char * host = NULL;
    char * port = NULL;
    host = strtok(tok, ":");
    port = strtok(NULL, ":");
    if (port == NULL) {
        port = PORT;
    }
    if (path == NULL) {
        path = "index.html";
    }

	if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    char buffer[200];
    sprintf(buffer, "GET /%s HTTP/1.0\r\n\r\n", path);
    send(sockfd, buffer,strlen(buffer),0);
	memset(buf,0,MAXDATASIZE-1);
    numbytes = -1;
    FILE * f = fopen("output","wb");
    int header_done = 0;
    char * ptr;
    char * sizeofbuf;
    while (numbytes != 0) {
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	        perror("recv");
	        exit(1);
	    }
        printf("%s",buf);
        if (!header_done) {
            ptr = strstr(buf,"\r\n\r\n");
        }
        if (ptr != NULL) {
            header_done = 1;
            sizeofbuf = buf;
            buf = ptr + 4;
        }
        if (header_done) {
            //fwrite(buf,sizeof(char),numbytes-(ptr+4-sizeofbuf),f);
            fprintf(f,"%s",ptr+4);
        }
        memset(buf,0,MAXDATASIZE-1);
	    buf[MAXDATASIZE] = '\0';
    }
	
    close(sockfd);
    // strip header from file

    fclose(f);

	return 0;
}

