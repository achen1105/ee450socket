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

    // TXLIST
    if (argc == 2 && strcmp(argv[1],"TXLIST") == 0)
    {
        // SEND MESSAGE TO SERVER
        if (send(sockfd, "clientA send TXLIST to serverM", strlen("clientA send TXLIST to serverM"), 0) == -1)
        {
            perror("send");
        }
        printf("%s sent a TXLIST request to the main server.\n", argv[1]);

        // CLIENT RECEIVES MESSAGE FROM SERVER THEN DONE
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // ending null char
        printf("clientA: received '%s'\n",buf);
        printf("TXLIST is generated.");
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
        printf("clientA: received '%s'\n",buf);
        string balance_amount = buf;
        balance_amount = balance_amount.substr(3, string::npos);
        printf("The current balance of %s is: %s alicoins.", argv[1], balance_amount.c_str());
    }
    // STATS
    else if (argc == 3 && strcmp(argv[2],"stats") == 0)
    {
        // SEND MESSAGE TO SERVER
        if (send(sockfd, "clientA send STATS to serverM", strlen("clientA send STATS to serverM"), 0) == -1)
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
        printf("clientA: received '%s'\n",buf);
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
        printf("clientA: received '%s'\n",buf);

        string balance(buf);
        balance = balance.substr(6, string::npos);

        // TXCOINS SCENARIOS
        if (buf[3]=='1' && buf[4] == '0') // code 1 for txcoins, 0 for successful txcoins
        {
            // successful TXCOINS
            printf("%s successfully transferred %s alicoins to %s.\nThe current balance of %s is :%s alicoins.", argv[1], argv[3], argv[2], argv[1], balance.c_str());
        }
        else if (buf[3]=='1' && buf[4] == '1') // code 1 for txcoins, 1 for insufficient balance
        {
            // insufficient balance
            printf("%s was unable to transfer %s alicoins to %s because of insufficient balance. The current balance of %s is :<BALANCE_AMOUNT> alicoins.", argv[1], argv[3], argv[2], argv[1]);
        }
        else if (buf[3]=='1' && buf[4] == '2') // code 1 for txcoins, 1 client not in network
        {
            // 1 not in network
            printf("Unable to proceed with the transaction as <SENDER_USERNAME/RECEIVER_USERNAME> is not part of the network.");
        }
        else if (buf[3]=='1' && buf[4] == '3') // code 1 for txcoins, 2 clients not in network
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
