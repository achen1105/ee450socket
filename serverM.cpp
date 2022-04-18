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

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100 // max number of bytes we can get at once (from TCP clients)

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
    // CREATE TCP SOCKET 1
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
    // for receiving messages from TCP sockets
    int numbytes; // check message length
    char buf[MAXDATASIZE]; // store message

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo("127.0.0.1", PORTCA, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
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
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
            
            // START TALKING HERE

            // WAIT FOR CLIENT COMMAND
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
            {
                perror("recv");
	        }
            buf[numbytes] = '\0'; // ending null char
            printf("serverM: received '%s'\n",buf);

            // SEND REQUESTED INFO MESSAGE TO CLIENT
			if (send(new_fd, "10 serverM send req info to clientA", strlen("10 serverM send req info to clientA"), 0) == -1)
            {
                perror("send");
            }
             printf("serverM: send '%s'\n", "10 serverM send req info to clientA");

            // DONE TALKING HERE
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
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
