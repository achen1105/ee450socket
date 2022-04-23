/*
** client.c -- a stream socket client demo
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
#include <string>

using namespace std;

#define PORT "25421" // the TCP port client will be connecting to 
#define MAXDATASIZE 1500 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
/**
 * @brief Get the in addr object
 * 
 * @param sa socket address
 * @return void* 
 */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * @brief main function
 * 
 * @param argc 2 for TXLIST or CHECKWALLET, 3 for stats, 4 for TXCOINS
 * @param argv <username> or <username1> <username2> <transfer amount> 
 * or <username> stats or TXLIST
 * @return int 0 if successful
 */
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;

    int numbytes;  
	char buf[MAXDATASIZE];
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    // save address info in rv
	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) 
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        // create socket
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
			perror("client: socket");
			continue;
		}

        // connect socket
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

    // check if socket is connected
	if (p == NULL) 
    {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    printf("The client A is up and running.\n"); // our own "client connecting to" message
    // CLIENT DONE WITH ALL CONNECTING HERE

    // START TALKING WITH OTHERS HERE

    // TXLIST, code TL
    if (argc == 2 && strcmp(argv[1],"TXLIST") == 0)
    {
        // SEND MESSAGE TO SERVER
        if (send(sockfd, "TL", strlen("TL"), 0) == -1)
        {
            perror("send");
        }
        printf("%s sent a sorted list request to the main server.\n", argv[1]);

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        //printf("clientA: received '%s'\n",buf);
        printf("Sorted list is generated.");
    }
    // CHECK WALLET; code CW for check wallet
    else if (argc == 2)
    {
        string username(argv[1]);
        string cwmsg1 = "CW " + username;
        // SEND MESSAGE TO SERVER
        if (send(sockfd, cwmsg1.c_str(), strlen(cwmsg1.c_str()), 0) == -1)
        {
            perror("send");
        }
        printf("%s sent a balance enquiry request to the main server.\n", argv[1]);

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        //printf("clientA: received '%s'\n",buf);
        string balance_amount = buf;
        if (balance_amount.compare("F") == 0)
        {
            printf("Unable to proceed with the transaction as %s is not part of the network.\n", argv[1]);
        }
        else
        {
            printf("The current balance of %s is: %s alicoins.\n", argv[1], balance_amount.c_str());
        }
    }
    // STATS code ST
    else if (argc == 3 && strcmp(argv[2],"stats") == 0)
    {
        // SEND MESSAGE TO SERVER
        string username(argv[2]);
        string stmsg1 = "ST " + username;
        if (send(sockfd, stmsg1.c_str(), strlen(stmsg1.c_str()), 0) == -1)
        {
            perror("send");
        }
        printf("%s sent a statistics balance enquiry request to the main server.\n", argv[1]);

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        //printf("clientA: received '%s'\n",buf);
        printf("%s statistics are the following.:”Rank--Username--NumofTransacions--Total\n", argv[1]);
    }
    // TXCOINS CODE TC
    else if (argc == 4)
    {
        string username1 = argv[1];
        string username2 = argv[2];
        string amt = argv[3];
        string tcmsg1 = "TC " + username1 + " " + username2 + " " + amt;

        // SEND MESSAGE TO SERVER
        if (send(sockfd, tcmsg1.c_str(), strlen(tcmsg1.c_str()), 0) == -1)
        {
            perror("send");
        }
        printf("%s has requested to transfer %s coins to %s.\n", argv[1], argv[3], argv[2]);

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE SO EXIT
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        // printf("clientA: received '%s'\n",buf);

        // TXCOINS SCENARIOS
        if (buf[3]=='S' && buf[4] == 'C') // code SC successful transaction
        {
            // successful TXCOINS
            string bal(buf);
            bal = bal.substr(6, string::npos);
            printf("%s successfully transferred %s alicoins to %s.\nThe current balance of %s is: %s alicoins.\n", argv[1], argv[3], argv[2], argv[1], bal.c_str());
        }
        else if (buf[3]=='I' && buf[4] == 'B') // code IB insufficient balance
        {
            // insufficient balance
            string bal(buf);
            bal = bal.substr(6, string::npos);
            printf("%s was unable to transfer %s alicoins to %s because of insufficient balance. The current balance of %s is: %s alicoins.\n", argv[1], argv[3], argv[2], argv[1], bal.c_str());
        }
        else if (buf[3]=='O' && buf[4] == 'N') // code ON one client not in network
        {
            // 1 not in network
            string userNAN(buf);
            userNAN = userNAN.substr(6, string::npos);
            printf("Unable to proceed with the transaction as %s is not part of the network.", userNAN.c_str());
        }
        else if (buf[3]=='B' && buf[4] == 'N') // code BN both clients not in network
        {
            // 2 not in network
            printf("Unable to proceed with the transaction as %s and %s are not part of the network.", argv[1], argv[2]);
        }
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
