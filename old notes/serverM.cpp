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
#include <iostream>

#define MYNODE "127.0.0.1" // local host hardcoded
#define MYPORT "24421"    // the UDP port users will be connecting to
#define MAXBUFLEN 65536 // max buffer length

#define PORT2 "25421"  // the TCP port users will be connecting to for client A
#define BACKLOG2 10 // how many pending connections queue will hold

#define PORT3 "26421"  // the TCP port users will be connecting to for client B
#define BACKLOG3 10 // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Adapted from 6.1 A Simple Stream Server
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

/*
** listener.c -- a datagram sockets "server" demo
*/
int createSockets()
{
    // UDP Socket
    int sockfdUDP;
    struct addrinfo hintsUDP, *servinfoUDP, *pUDP;
    int rvUDP;
    int numbytesUDP;
    struct sockaddr_storage their_addrUDP;
    char bufUDP[MAXBUFLEN];
    socklen_t addr_lenUDP;
    char sUDP[INET6_ADDRSTRLEN];

    memset(&hintsUDP, 0, sizeof hintsUDP);
    hintsUDP.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hintsUDP.ai_socktype = SOCK_DGRAM;
    //hintsUDP.ai_flags = O_NONBLOCK;

    // check errors in address info
    if ((rvUDP = getaddrinfo(MYNODE, MYPORT, &hintsUDP, &servinfoUDP)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvUDP));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(pUDP = servinfoUDP; pUDP != NULL; pUDP = pUDP->ai_next) {
        if ((sockfdUDP = socket(pUDP->ai_family, pUDP->ai_socktype,
                pUDP->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfdUDP, pUDP->ai_addr, pUDP->ai_addrlen) == -1) {
            close(sockfdUDP);
            perror("listener: bind");
            continue;
        }

        break;
    }
    
    // error in binding socket
    if (pUDP == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfoUDP);
    
    //close(sockfd);
    //return 0;

    // TCP socket 1
    int sockfdTCP1, new_fdTCP1;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hintsTCP1, *servinfoTCP1, *pTCP1;
    struct sockaddr_storage their_addrTCP1; // connector's address information
    socklen_t sin_sizeTCP1;
    // struct sigaction saTCP1;
    int yesTCP1=1;
    char sTCP1[INET6_ADDRSTRLEN];
    int rvTCP1;

    int numbytesTCP1;
    char bufTCP1[MAXBUFLEN];

    memset(&hintsTCP1, 0, sizeof hintsTCP1);
    hintsTCP1.ai_family = AF_UNSPEC;
    hintsTCP1.ai_socktype = SOCK_STREAM;
    hintsTCP1.ai_flags = AI_PASSIVE; // use my IP

    if ((rvTCP1 = getaddrinfo(MYNODE, PORT2, &hintsTCP1, &servinfoTCP1)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvTCP1));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(pTCP1 = servinfoTCP1; pTCP1 != NULL; pTCP1 = pTCP1->ai_next) {
        if ((sockfdTCP1 = socket(pTCP1->ai_family, pTCP1->ai_socktype,
                pTCP1->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfdTCP1, SOL_SOCKET, SO_REUSEADDR, &yesTCP1,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfdTCP1, pTCP1->ai_addr, pTCP1->ai_addrlen) == -1) {
            close(sockfdTCP1);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfoTCP1); // all done with this structure

    if (pTCP1 == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfdTCP1, BACKLOG2) == -1) {
        perror("listen");
        exit(1);
    }

    /**
    // interferes with select(), commented out 
    saTCP1.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&saTCP1.sa_mask);
    saTCP1.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &saTCP1, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    */

    // successful three sockets
    printf("The main server is up and running.\n");
    //printf("server: waiting for connections...\n");

    // 7.3 and https://stackoverflow.com/questions/15560336/listen-to-multiple-ports-from-one-server
    

    // keeps UDP/TCP running
    while(1)
    {
        fd_set master;
        FD_ZERO(&master);
        FD_SET(sockfdUDP, &master);
        FD_SET(sockfdTCP1, &master);
        int fd_max = sockfdUDP;
        if (sockfdTCP1 > sockfdUDP)
        {
            fd_max = sockfdTCP1;
        }

        fd_set read_fds = master; // copy
        if (select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            std::cout<< fd_max << std::endl;
            perror("select");
            exit(4);
        }

        // use select() to decide which port to connect to
        for (int i = 0; i <= fd_max; i++)
        {
            if (FD_ISSET(i, &read_fds)) // we got one!!
            { 
                if (i == sockfdUDP) 
                {
                    // UDP SOCKET
                    // check buffer length
                    addr_lenUDP = sizeof their_addrUDP;
                    if ((numbytesUDP = recvfrom(sockfdUDP, bufUDP, MAXBUFLEN-1 , 0,
                        (struct sockaddr *)&their_addrUDP, &addr_lenUDP)) == -1) {
                        perror("recvfrom");
                        exit(1);
                    }

                    printf("listener: got packet from %s\n",
                        inet_ntop(their_addrUDP.ss_family,
                            get_in_addr((struct sockaddr *)&their_addrUDP),
                            sUDP, sizeof sUDP));
                    printf("listener: packet is %d bytes long\n", numbytesUDP);
                    bufUDP[numbytesUDP] = '\0';
                    printf("listener: packet contains \"%s\"\n", bufUDP);

                    
                        // port and address for server A
                        int rv, numbytes;
                        struct addrinfo *servinfoSA;
                        if ((rv = getaddrinfo(MYNODE, "21421", NULL, &servinfoSA)) != 0) {
                            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                            return 1;
                        }
                        
                        if ((numbytes=sendto(sockfdUDP, "hello", strlen("hello"), 0, servinfoSA->ai_addr, servinfoSA->ai_addrlen) == -1))
                        {
                            perror("Server M: sendto");
                            exit(1);
                        }
                        
                        /**addr_lenUDP = sizeof their_addrUDP;
                        if ((numbytesUDP = sendto(sockfdUDP, "hello", strlen("hello"), 0, (struct sockaddr *)&their_addrUDP, addr_lenUDP))== -1)
                        {
                            perror("Server M: sendto");
                            exit(1);
                        }*/
                }
                // CHECK WALLET
                else if (i == sockfdTCP1)
                {
                    // TCP SOCKET 1
                    sin_sizeTCP1 = sizeof their_addrTCP1;
                    new_fdTCP1 = accept(sockfdTCP1, (struct sockaddr *)&their_addrTCP1, &sin_sizeTCP1);
                    if (new_fdTCP1 == -1) {
                        perror("accept");
                        continue;
                    }

                    inet_ntop(their_addrTCP1.ss_family,
                        get_in_addr((struct sockaddr *)&their_addrTCP1),
                        sTCP1, sizeof sTCP1);
                    printf("server: got connection from %s\n", sTCP1);

                    if (!fork()) 
                    { // this is the child process
                        close(sockfdTCP1); // child doesn't need the listener
                        // int send(int sockfd, const void *msg, int len, int flags); 
                        if ((numbytesTCP1 = recv(new_fdTCP1, bufTCP1, MAXBUFLEN-1 , 0) == -1)) 
                        {
                            perror("recv");
                            exit(1);
                        }
                        printf("The main server received input = '%s' from the client using TCP over port '%s'.\n",bufTCP1, PORT2);
                        
                        
                        

                        close(new_fdTCP1);
                        exit(0);

                        /**
                        // example
                        if (send(new_fdTCP1, "Hello, world!", 13, 0) == -1)
                            perror("send");
                        close(new_fdTCP1);
                        exit(0);
                        */
                    }
                    close(new_fdTCP1);  // parent doesn't need this
                }
                else
                {
                    perror("accept");
                }
            }
        }
    }
    
    //return 0;
}

int main(void)
{
    createSockets();
}