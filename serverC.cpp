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
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#define SERVERMPORT "24421"	// the port users will be connecting to (destination)
#define SERVERCPORT "23421" // the source port
#define MAXDATASIZE 1500 // max number of bytes we can get at once (from TCP clients and UDP clients)

// https://www.cplusplus.com/doc/tutorial/files/
// https://stackoverflow.com/questions/20372661/read-word-by-word-from-file-in-c
int checkWallet(string usrnme)
{
	string line;
	int tNum;
	string tUsr1;
	string tUsr2;
	int tAmt;

	int balance = 0;
	ifstream myfile ("block3.txt");

	if (myfile.is_open())
	{
		while (myfile >> tNum >> tUsr1 >> tUsr2 >> tAmt)
		{
			// username is sender
			if (usrnme.compare(tUsr1) == 0)
			{
				balance = balance - tAmt;
			}
			// username is receiver
			else if (usrnme.compare(tUsr2) == 0)
			{
				balance = balance + tAmt;
			}
		}

		myfile.close();
	}

	else cout << "Unable to open file"; 
	return balance;
}

// "T" if in network, "F" if not in network
string checkUser(string usrnme)
{
	string line;
	int tNum;
	string tUsr1;
	string tUsr2;
	int tAmt;

	ifstream myfile ("block3.txt");

	if (myfile.is_open())
	{
		while (myfile >> tNum >> tUsr1 >> tUsr2 >> tAmt)
		{
			// username is sender
			if (usrnme.compare(tUsr1) == 0)
			{
				return "T";
			}
			// username is receiver
			else if (usrnme.compare(tUsr2) == 0)
			{
				return "T";
			}
		}

		myfile.close();
	}

	else cout << "Unable to open file"; 
	return "F";
}

string txList()
{
	ifstream myfile ("block3.txt");
	string line;
	string list = "";

	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			list = list + line + "\n";
		}

		myfile.close();
	}

	else cout << "Unable to open file"; 
	return list;
}

int main(int argc, char *argv[])
{
	int sockfd; // main serverA socket to send and receive
	int numbytes; // check bytes in message
    char buf[MAXDATASIZE]; // store received messages
	struct sockaddr_in servMaddr,servCaddr; // servMaddr the server M addr info, servAaddr the server A addr info
	// create socket
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	// server M addr info
	servMaddr.sin_family = AF_INET;
	socklen_t servMaddr_len;
	servMaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servMaddr.sin_port=htons(24421); //destination port for incoming packets

	// server C addr info
	servCaddr.sin_family = AF_INET;
	servCaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	servCaddr.sin_port=htons(23421); //source port for outgoing packets
	
	// bind main server C socket to server C addr info
	if (bind(sockfd,(struct sockaddr *)&servCaddr,sizeof(servCaddr)))
	{
		perror("bind");
	}
	
	printf("The ServerC is up and running using UDP on port %s.\n", SERVERCPORT);

	// TESTING AND CONNECTING TO SERVER M
	if ((numbytes = sendto(sockfd, "test", strlen("test"), 0,
			 (struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
	{
		perror("server C client socket: sendto");
		exit(1);
	}
	//printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // receive request from serverM
    // always put following line before recvfrom
    servMaddr_len = sizeof servMaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servMaddr, &servMaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverC: received '%s'\n", buf);

    // send req info to serverM
    if ((numbytes = sendto(sockfd, "req info", strlen("req info"), 0,
            (struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
    {
        perror("server C client socket: sendto");
        exit(1);
    }
    // printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // WAITING FOR MESSAGES FROM SERVER M
	while (1)
	{
		// receive request from serverM
		// always put following line before recvfrom
		servMaddr_len = sizeof servMaddr;
		if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
			(struct sockaddr *) &servMaddr, &servMaddr_len)) == -1) 
		{
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
		//printf("serverC: received '%s'\n", buf);
		printf("The ServerC received a request from the Main Server.\n");

		// CHECK WALLET code CW
		if (buf[0] == 'C' && buf[1] == 'W')
		{
			string username(buf);
			username = username.substr(3, string::npos);
			string usernameBalance = checkUser(username) + " " + to_string(checkWallet(username));

			// send req info to serverM
			if ((numbytes = sendto(sockfd, usernameBalance.c_str(), strlen(usernameBalance.c_str()), 0,
					(struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
			{
				perror("server C client socket: sendto");
				exit(1);
			}
			//printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");
			printf("The ServerC finished sending the response to the Main Server.\n");
		}
		// TXCOINS TC
		else if (buf[0] == 'T' && buf[1] == 'C')
		{
			string transaction(buf);
			transaction = transaction.substr(3, string::npos);
			string username = transaction.substr(0, transaction.find(" "));
			string tcmsg2 = "TC 10 " + to_string(checkWallet(username));

			// send req info to serverM
			if ((numbytes = sendto(sockfd, tcmsg2.c_str(), strlen(tcmsg2.c_str()), 0,
					(struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
			{
				perror("server C client socket: sendto");
				exit(1);
			}
			printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");
			printf("The ServerC finished sending the response to the Main Server.\n");
		}
		// TXLIST TL
		else if (buf[0] == 'T' && buf[1] == 'L')
		{
			// send req info to serverM
			if ((numbytes = sendto(sockfd, txList().c_str(), strlen(txList().c_str()), 0,
					(struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
			{
				perror("server C client socket: sendto");
				exit(1);
			}
			printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");
			printf("The ServerC finished sending the response to the Main Server.\n");
		}
		// STATS ST
		else if (buf[0] == 'S' && buf[1] == 'T')
		{
			// send req info to serverM
			if ((numbytes = sendto(sockfd, "ranking", strlen("ranking"), 0,
					(struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
			{
				perror("server C client socket: sendto");
				exit(1);
			}
			printf("server C: sent %d bytes to %s\n", numbytes, "127.0.0.1");
			printf("The ServerC finished sending the response to the Main Server.\n");
		}
		else
		{
			// extra check wallet and check user to go with private method in serverM
			string username(buf);
			string usernameBalance = checkUser(username) + " " + to_string(checkWallet(username));

			// send req info to serverM
			if ((numbytes = sendto(sockfd, usernameBalance.c_str(), strlen(usernameBalance.c_str()), 0,
					(struct sockaddr *) &servMaddr, sizeof(servMaddr))) == -1) 
			{
				perror("server A client socket: sendto");
				exit(1);
			}
			//printf("server A: sent %d bytes to %s\n", numbytes, "127.0.0.1");
			printf("The ServerA finished sending the response to the Main Server.\n");
		}
	}
	
	return 0;
}