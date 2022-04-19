/*
** talker.c -- a datagram "client" demo
https://stackoverflow.com/questions/9873061/how-to-set-the-source-port-in-the-udp-socket-in-c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVERMPORT "24421"	// the port users will be connecting to (destination)
#define SERVERAPORT "21421" // the source port
#define MAXDATASIZE 100 // max number of bytes we can get at once (from TCP clients and UDP clients)


int main(int argc, char *argv[])
{
	int sockfd; // main serverA socket to send and receive
	int numbytes; // check bytes in message
    char buf[MAXDATASIZE]; // store received messages
	struct sockaddr_in servMaddr,servAaddr; // servMaddr the server M addr info, servAaddr the server A addr info
	// create socket
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	// server M addr info
	servMaddr.sin_family = AF_INET;
	socklen_t servMaddr_len;
	servMaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servMaddr.sin_port=htons(24421); //destination port for incoming packets

	// server A addr info
	servAaddr.sin_family = AF_INET;
	servAaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	servAaddr.sin_port=htons(21421); //source port for outgoing packets
	
	// bind main server A socket to server A addr info
	if (bind(sockfd,(struct sockaddr *)&servAaddr,sizeof(servAaddr)))
	{
		perror("bind");
	}
	
	printf("The Server A is up and running using UDP on port %s.\n", SERVERAPORT);

	// test receive

	// always put following line before recvfrom
	servMaddr_len = sizeof servMaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servMaddr, &servMaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("serverA: received '%s'\n", buf);

	// SEND TO SERVER M
	if ((numbytes = sendto(sockfd, "test", strlen("test"), 0,
			 (struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
	{
		perror("server A client socket: sendto");
		exit(1);
	}
	printf("server A: sent %d bytes to %s\n", numbytes, "127.0.0.1");

	return 0;
}