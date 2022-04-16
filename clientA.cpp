#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// 6.2 A Simple Stream Client
/*
** talker.c -- a datagram "client" demo
*/

#define SERVERPORT "24421"    // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    printf("The clientA is up and running.\n");

    if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    return 0;
}


/**
int main(int argc, char** argv)
{
    // 5.1
    struct addrinfo hints, *res;
    int sockfd;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    //int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
     //               const char *service,  // e.g. "http" or port number
       //             const struct addrinfo *hints,
         //           struct addrinfo **res);
    
    getaddrinfo("127.0.0.1", "24421", &hints, &res);

    // make a socket:
    // int socket(int domain, int type, int protocol); 
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // connect!
    // int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    cout << "The client A is up and running." << endl;
}
*/