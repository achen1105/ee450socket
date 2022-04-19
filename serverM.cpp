/*
** server.c -- a stream socket server demo
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

#define PORTCA "25421"  // the port users will be connecting to for client A
#define PORTCB "26421"  // the port users will be connecting to for client A
#define PORTSM "24421" // UDP port for server M 

#define BACKLOG 10	 // how many pending connections queue will hold (TCP clients)
#define MAXDATASIZE 100 // max number of bytes we can get at once (from TCP clients and UDP clients)

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{   
    int sockfd; // main serverM socket
    int numbytes; // number of bytes in message
    char buf[MAXDATASIZE]; // buffer to store recvfrom messages

    // create serverM socket
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    // SERVER M INFO
    struct sockaddr_in servMaddr;
    servMaddr.sin_family = AF_INET;
    servMaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servMaddr.sin_port = htons(24421); // Now the server will listen on PORT.

    // SERVER A INFO
    struct sockaddr_in servAaddr;
    socklen_t servAaddr_len;
    servAaddr.sin_family = AF_INET;
	servAaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	servAaddr.sin_port=htons(21421); //source port for outgoing packets

    // bind serverM socket to serverM addr info
    if(bind(sockfd, (struct sockaddr*)&servMaddr, sizeof(servMaddr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    printf("serverM: waiting to recvfrom...\n");
    // close(sockfd); keep on

    // DONE CREATING UDP SOCKET

    // test send to server A
    if ((numbytes = sendto(sockfd, "test", strlen("test"), 0,
			 (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
	{
		perror("server M to serverA: sendto");
		exit(1);
	}
	printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // WAIT FOR SERVERA TO SEND MESSAGE
    
    // always put following line before recvfrom
    servAaddr_len = sizeof servAaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("serverM: received '%s'\n", buf);

    // END UDP SOCKET

    // CREATE TCP SOCKET 1
	int sockfd1, new_fd1;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints1, *servinfo1, *p1;
	struct sockaddr_storage their_addr1; // connector's address information
	socklen_t sin_size1;
	struct sigaction sa1;
	int yes1=1;
	char s1[INET6_ADDRSTRLEN];
	int rv1;
    // for receiving messages from TCP sockets
    int numbytes1; // check message length
    char buf1[MAXDATASIZE]; // store message

	memset(&hints1, 0, sizeof hints1);
	hints1.ai_family = AF_UNSPEC;
	hints1.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv1 = getaddrinfo("127.0.0.1", PORTCA, &hints1, &servinfo1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv1));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p1 = servinfo1; p1 != NULL; p1 = p1->ai_next) {
		if ((sockfd1 = socket(p1->ai_family, p1->ai_socktype,
				p1->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &yes1,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd1, p1->ai_addr, p1->ai_addrlen) == -1) {
			close(sockfd1);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo1); // all done with this structure

	if (p1 == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd1, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa1.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa1.sa_mask);
	sa1.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa1, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
    // END MAIN TCP SOCKET 1, NOW LISTENING

    // CREATE TCP SOCKET 2 FOR LISTENING FOR CLIENT B
	int sockfd2, new_fd2;  // listen on sock_fd2, new connection on new_fd2
	struct addrinfo hints2, *servinfo2, *p2;
	struct sockaddr_storage their_addr2; // connector's address information
	socklen_t sin_size2;
	struct sigaction sa2;
	int yes2=1;
	char s2[INET6_ADDRSTRLEN];
	int rv2;
    // for receiving messages from TCP sockets
    int numbytes2; // check message length
    char buf2[MAXDATASIZE]; // store message

	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_UNSPEC;
	hints2.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv2 = getaddrinfo("127.0.0.1", PORTCB, &hints2, &servinfo2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
		if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
				p2->ai_protocol)) == -1) {
			perror("serverM: socket");
			continue;
		}

		if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &yes2,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
			close(sockfd2);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo2); // all done with this structure

	if (p2 == NULL)  {
		fprintf(stderr, "serverM: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd2, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa2.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa2.sa_mask);
	sa2.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa2, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
    // END MAIN TCP SOCKET 2, NOW LISTENING
    
	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
        // LISTEN FOR CLIENT A
		sin_size1 = sizeof their_addr1;
		new_fd1 = accept(sockfd1, (struct sockaddr *)&their_addr1, &sin_size1);
		if (new_fd1 == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr1.ss_family,
			get_in_addr((struct sockaddr *)&their_addr1),
			s1, sizeof s1);
		printf("server: got connection from %s\n", s1);

		if (!fork()) { // this is the child process
			close(sockfd1); // child doesn't need the listener
            
            // START TALKING HERE

            // WAIT FOR CLIENT COMMAND
            if ((numbytes1 = recv(new_fd1, buf1, MAXDATASIZE-1, 0)) == -1) 
            {
                perror("recv");
	        }
            buf1[numbytes1] = '\0'; // ending null char
            printf("serverM: received '%s'\n",buf1);

            // TALK TO SERVER A

            // SEND REQUESTED INFO MESSAGE TO CLIENT
			if (send(new_fd1, "10 serverM send req info to clientA", strlen("10 serverM send req info to clientA"), 0) == -1)
            {
                perror("send");
            }
             printf("serverM: send '%s'\n", "10 serverM send req info to clientA");

            // DONE TALKING HERE
			close(new_fd1);
			exit(0);
		}
		close(new_fd1);  // parent doesn't need this
        // END TCP ACCEPT FOR CLIENT A

        // LISTEN FOR CLIENT B
        sin_size2 = sizeof their_addr2;
		new_fd2 = accept(sockfd2, (struct sockaddr *)&their_addr2, &sin_size2);
		if (new_fd2 == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr2.ss_family,
			get_in_addr((struct sockaddr *)&their_addr2),
			s2, sizeof s2);
		printf("server: got connection from %s\n", s2);

		if (!fork()) { // this is the child process
			close(sockfd2); // child doesn't need the listener
            
            // START TALKING HERE

            // WAIT FOR CLIENT COMMAND
            if ((numbytes2 = recv(new_fd2, buf2, MAXDATASIZE-1, 0)) == -1) 
            {
                perror("recv");
	        }
            buf2[numbytes2] = '\0'; // ending null char
            printf("serverM: received '%s'\n",buf2);

            // SEND REQUESTED INFO MESSAGE TO CLIENT
			if (send(new_fd2, "10 serverM send req info to clientB", strlen("10 serverM send req info to clientB"), 0) == -1)
            {
                perror("send");
            }
             printf("serverM: send '%s'\n", "10 serverM send req info to clientB");

            // DONE TALKING HERE
			close(new_fd2);
			exit(0);
		}
		close(new_fd2);  // parent doesn't need this
        // END TCP ACCEPT TO CLIENT B
	}

	return 0;
}
