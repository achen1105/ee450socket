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
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <fstream>
#include <iostream>

// Adapted from 6.1 A Simple Stream Server
/*
** listener.c -- a datagram sockets "server" demo
*/

#define MYNODE "127.0.0.1"
#define MYPORT "21421"    // the port users will be connecting to

#define MAXBUFLEN 65536

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// https://www.cplusplus.com/doc/tutorial/files/
std::string getBlock1()
{
    std::string line;
    std::string output = "";
    std::ifstream myfile ("block1.txt");
    if (myfile.is_open())
    {
        while ( getline(myfile,line) )
        {
            std::cout << line << '\n';
            output = output + line;
        }
        myfile.close();
    }

    else std::cout << "Unable to open file"; 

    return output;
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP

    // port and address for server A
    if ((rv = getaddrinfo(MYNODE, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("talker: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("The ServerA is up and running using UDP on port 21421.\n");
    getBlock1();

    // port and address for server M
     struct addrinfo *servinfo2;
     if ((rv = getaddrinfo(MYNODE, "24421", NULL, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    if ((numbytes = sendto(sockfd, "hi", strlen("hi"), 0, servinfo2->ai_addr, servinfo2->ai_addrlen) == -1)) 
    {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo2);

    printf("talker: sent %d bytes to %s\n", numbytes, "127.0.0.1");
    close(sockfd);

    //close(sockfd);

    return 0;
}