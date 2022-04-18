/*
** talker.c -- a datagram "client" demo
*/

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

#define SERVERPORT "24421"	// the port users will be connecting to
#define PORTSA "21421"	// port that serverA listens on

int main(int argc, char *argv[])
{
	// CREATE SOCKET FOR SERVER A ON PORT 21421
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	// get info for port 21421
	if ((rv = getaddrinfo("127.0.0.1", PORTSA, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket and bind
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server A: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server A: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server A: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("The ServerA is up and running using UDP on port %s.", PORTSA);
	// DONE WITH CREATING SOCKET FOR SERVER A

	// FIND DESTINATION ADDRESS INFO FOR SERVER M
	int sockfdM;
	struct addrinfo hintsM, *servinfoM, *pM;
	int rvM;
	int numbytesM;

	memset(&hintsM, 0, sizeof hintsM);
	hintsM.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hintsM.ai_socktype = SOCK_DGRAM;

	// get info for port 24421
	if ((rvM = getaddrinfo("127.0.0.1", SERVERPORT, &hintsM, &servinfoM)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvM));
		return 1;
	}

	// loop through all the results and make a socket (just to get dest addr for server M)
	for(pM = servinfoM; pM != NULL; pM = pM->ai_next) {
		if ((sockfdM = socket(pM->ai_family, pM->ai_socktype,
				pM->ai_protocol)) == -1) {
			perror("server A dest: socket");
			continue;
		}

		break;
	}

	if (pM == NULL) {
		fprintf(stderr, "server A dest: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfoM);

	// SENDTO AND RECVFROM MESSAGES

	// send initial message to server M
	if ((numbytesM = sendto(sockfdM, "serverA to serverM init", strlen("serverA to serverM init"), 0,
			 pM->ai_addr, pM->ai_addrlen)) == -1) {
		perror("server A: sendto");
		exit(1);
	}
	printf("server A: sent %d bytes to %s\n", numbytesM, "127.0.0.1");

	//close(sockfd); // keep it always on

	return 0;
}