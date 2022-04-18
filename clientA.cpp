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

#define PORT "25421" // the port client will be connecting to 

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
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

    // WILL BE HANDLED LATER
    /**
	if (argc != 2) {
	    fprintf(stderr,"usage: client <username1>\n");
	    exit(1);
	}
    */

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
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

        // CONNECT ONCE HERE
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
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
	// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    printf("The client A is up and running.\n"); // our own "client connecting to"
    // CLIENT DONE WITH ALL CONNECTING HERE

    // START TALKING HERE

    // CHECK WALLET
    if (argc == 2)
    {
        // SEND MESSAGE TO SERVER
        if (send(sockfd, "clientA send CHECKWALLET to serverM", strlen("clientA send CHECKWALLET to serverM"), 0) == -1)
        {
            perror("send");
        }
        printf("clientA: send '%s'\n", "clientA send CHECKWALLET to serverM");

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        printf("clientA: received '%s'\n",buf);
    }
    // TXCOINS
    else if (argc == 4)
    {
        // SEND MESSAGE TO SERVER
        if (send(sockfd, "clientA send TXCOINS to serverM", strlen("clientA send TXCOINS to serverM"), 0) == -1)
        {
            perror("send");
        }
        printf("clientA: send '%s'\n", "clientA send TXCOINS to serverM");

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE SO EXIT
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        printf("clientA: received '%s'\n",buf);
    }
    else
    {
        fprintf(stderr,"clientA usage: ./clientA <username1> or ./clientA <username1> <username2> <transfer amount>\n");
	    exit(1);
    }

    // DONE TALKING HERE
    
    // close the one socket
	close(sockfd);

	return 0;
}

void checkWalletClient()
{

}
