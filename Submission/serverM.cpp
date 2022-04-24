/*
** server.c -- a stream socket server demo https://beej.us/guide/bgnet/html/
    ONLY REFERENCED THE STRUCTURE
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
#include <sstream>
#include <map>
#include <utility>

using namespace std;

#define PORTCA "25421"  // the port users will be connecting to for client A
#define PORTCB "26421"  // the port users will be connecting to for client B
#define PORTSM "24421"  // UDP port for server M 
#define PORTSA "21421"
#define PORTSB "22421"
#define PORTSC "23421"

#define BACKLOG 10	 // how many pending connections queue will hold (TCP clients)
#define MAXDATASIZE 1500 // max number of bytes we can get at once (from TCP clients and UDP clients)

// C++11 has stoi, vm does not
// https://stackoverflow.com/questions/19311641/c-string-to-int-without-using-atoi-or-stoi
// https://www.geeksforgeeks.org/converting-strings-numbers-cc/
int stoint(string s)
{
    int i = 0;

    stringstream temp(s);
    temp >> i;
    return i;
}

// C++11 has to_string, vm does not
// https://stackoverflow.com/questions/4668760/converting-an-int-to-stdstring
string to_string(int x, int y)
{
  // int y not used, just to suppress overload warnings
  int length = snprintf( NULL, 0, "%d", x );
  // assert( length >= 0 );
  char* buf = new char[length + 1];
  snprintf( buf, length + 1, "%d", x );
  std::string str( buf );
  delete[] buf;
  return str;
}

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

// https://www.geeksforgeeks.org/insertion-sort/
/* Function to sort an array using insertion sort*/
void insertionSort(string arr[], int n)
{
    int i, j;
    int key = 0;
    string temp;
    for (i = 1; i < n; i++)
    {
        key = stoint(arr[i].substr(0, arr[i].find_first_of(" ")));
        temp = arr[i];
        j = i - 1;
 
        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && stoint(arr[j].substr(0, arr[j].find_first_of(" "))) > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = temp;
    }
}

// https://www.geeksforgeeks.org/insertion-sort/
// Sort transactions
void insertionSortTransactions(string arr[], int n)
{
    int i, j;
    int key = 0;
    string temp;
    for (i = 1; i < n; i++)
    {   
        string middle = arr[i].substr(0, arr[i].find_last_of(" "));
        key = stoint(middle.substr(middle.find_first_of(" "), string::npos));
        temp = arr[i];
        j = i - 1;
 
        /* Move elements of arr[0..i-1], that are
        less than key, to one position ahead
        of their current position */
        string middleCheck = arr[j].substr(0, arr[j].find_last_of(" "));
        int middleCheckFinal = stoint(middleCheck.substr(middleCheck.find_first_of(" "), string::npos));
        while (j >= 0 &&  middleCheckFinal < key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = temp;
    }
}

// https://www.w3schools.com/cpp/cpp_files.asp
void writeTXLIST(string list, int size)
{
    string line;
    string lines[size];
    istringstream f(list);
    ofstream myfile("alichain.txt");
    int index = 0;

    while (getline(f, line))
    {
        lines[index] = line;
        // myfile << line << '\n';
        index++;
    }
    
    insertionSort(lines, size);

    for (int i = 0; i < size; i++)
    {
        myfile << lines[i] << '\n';
        //printf("%s \n", lines[i].c_str());
    }

    myfile.close();
}

string getStats(string list)
{
    string statsLine = "";
    int index = 0;
    string finalStats = "";

    map<string, int> count;
    map<string, int> balance;

    string username = "";
    int ct = 0;
    int bal = 0;

    istringstream f(list);

    // initialize map out of order
    while (f >> username >> ct >> bal)
    {
        // username is already in the map
        if (count.find(username) != count.end())
        {
            count[username] = count[username] + ct;
            balance[username] = balance[username] + bal;
        }
        // add username
        else
        {
            count[username] = ct;
            balance[username] = bal;
        }
    }
    string statsArray[count.size()];

    // https://www.cplusplus.com/reference/map/map/begin/
	for (map<string,int>::iterator it=count.begin(); it!=count.end(); ++it)
	{
		int currentBal = balance[it->first];
		// username, count, balance
		statsLine = it->first + " " + to_string(it->second, 0) + " " + to_string(currentBal, 0);
        statsArray[index] = statsLine;
        index++;
	}

    insertionSortTransactions(statsArray, count.size());

    for (int i = 0; i < count.size(); i++)
    {
        int rank = i + 1;
        finalStats = finalStats + to_string(rank, 0) + " " + statsArray[i] + "\n";
    }
    
    return finalStats;
}

// "F if not found, then int balance"
int findSequenceNumber(int sockfd, sockaddr_in servAaddr, socklen_t servAaddr_len, sockaddr_in servBaddr, socklen_t servBaddr_len, sockaddr_in servCaddr, socklen_t servCaddr_len)
{
    int maxSequence = 0;
    char buf[MAXDATASIZE];
    int numbytes;

     // send req to server A
    if ((numbytes = sendto(sockfd, "SQ", strlen("SQ"), 0,
			 (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
	{
		perror("server M to serverA: sendto");
		exit(1);
	}
	//printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // receive req info
    // always put following line before recvfrom
    servAaddr_len = sizeof servAaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';

    string tempSequenceA(buf);
    int maxSequenceA = stoint(tempSequenceA);
    if (maxSequenceA > maxSequence)
    {
        maxSequence = maxSequenceA;
    }

    // send req to server B
    if ((numbytes = sendto(sockfd, "SQ", strlen("SQ"), 0,
			 (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
	{
		perror("server M to serverB: sendto");
		exit(1);
	}
	//printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // receive req info
    // always put following line before recvfrom
    servBaddr_len = sizeof servBaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';

    string tempSequenceB(buf);
    int maxSequenceB = stoint(tempSequenceB);
    if (maxSequenceB > maxSequence)
    {
        maxSequence = maxSequenceB;
    }
    //printf("serverM: received '%s'\n", buf);

    // send req to server C
    if ((numbytes = sendto(sockfd, "SQ", strlen("SQ"), 0,
			 (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
	{
		perror("server M to serverC: sendto");
		exit(1);
	}
	//printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

    // receive req info
    // always put following line before recvfrom
    servCaddr_len = sizeof servCaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';

    string tempSequenceC(buf);
    int maxSequenceC = stoint(tempSequenceC);
    if (maxSequenceC > maxSequence)
    {
        maxSequence = maxSequenceC;
    }

    //printf("serverM: max sequence is '%s'\n", to_string(maxSequence).c_str());
    return maxSequence;
}

// "F if not found, then int balance", only use in TXCOINS
string checkWallet(int sockfd, string usr, sockaddr_in servAaddr, socklen_t servAaddr_len, sockaddr_in servBaddr, socklen_t servBaddr_len, sockaddr_in servCaddr, socklen_t servCaddr_len)
{
    int totalBalance = 0;
    int numbytes;
    char buf[MAXDATASIZE];
    //printf("checking wallet for %s\n", usr.c_str());

    // TALK TO SERVER A
    // send req to server A, put buf1 here because want to relay message from CA
    if ((numbytes = sendto(sockfd, usr.c_str(), strlen(usr.c_str()), 0,
            (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
    {
        perror("server M to serverA: sendto");
        exit(1);
    }
    //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
    printf("The main server sent a request to server A.\n");

    // receive req info
    // always put following line before recvfrom
    servAaddr_len = sizeof servAaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverM: received '%s'\n", buf);
    //printf("The main server received transactions from Server A using UDP over port %s.\n", PORTSA);
    string balanceA(buf);
    string statusA = balanceA.substr(0, 1);
    balanceA = balanceA.substr(2, string::npos);
    totalBalance = totalBalance + stoint(balanceA);
    printf("The main server received the feedback from server A using UDP over port %s.\n", PORTSA);

    // TALK TO SERVER B
    // send req to server B, put buf1 here because want to relay message from CA
    if ((numbytes = sendto(sockfd, usr.c_str(), strlen(usr.c_str()), 0,
            (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
    {
        perror("server M to serverB: sendto");
        exit(1);
    }
    //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
    printf("The main server sent a request to server B.\n");

    // receive req info
    // always put following line before recvfrom
    servBaddr_len = sizeof servBaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverM: received '%s'\n", buf);
    //printf("The main server received transactions from Server B using UDP over port %s.\n", PORTSB);
    string balanceB(buf);
    string statusB = balanceB.substr(0, 1);
    balanceB = balanceB.substr(2, string::npos);
    totalBalance = totalBalance + stoint(balanceB);
    printf("The main server received the feedback from server B using UDP over port %s.\n", PORTSB);

    // TALK TO SERVER C
    // send req to server C, put buf1 here because want to relay message from CA
    if ((numbytes = sendto(sockfd, usr.c_str(), strlen(usr.c_str()), 0,
            (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
    {
        perror("server M to serverC: sendto");
        exit(1);
    }
    //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
    printf("The main server sent a request to server C.\n");

    // receive req info
    // always put following line before recvfrom
    servCaddr_len = sizeof servCaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverM: received '%s'\n", buf);
    //printf("The main server received transactions from Server C using UDP over port %s.\n", PORTSC);
    string balanceC(buf);
    string statusC = balanceC.substr(0, 1);
    balanceC = balanceC.substr(2, string::npos);
    totalBalance = totalBalance + stoint(balanceC);
    printf("The main server received the feedback from server C using UDP over port %s.\n", PORTSC);

    // add initial balance
    totalBalance = 1000 + totalBalance;

    // cannot find person in network
    if (statusA.compare("F") == 0 && statusB.compare("F") == 0 && statusC.compare("F")== 0)
    {
        return "F 0";
    }
    return "T " + to_string(totalBalance, 0);
}

void serverMOperations(int sockfd, int sockfd1, int new_fd1, int numbytes, int numbytes1, char s1[], char buf[], char buf1[], socklen_t sin_size1, sockaddr_storage their_addr1, sockaddr_in servAaddr, socklen_t servAaddr_len, sockaddr_in servBaddr, socklen_t servBaddr_len, sockaddr_in servCaddr, socklen_t servCaddr_len, string client, string portNum)
{
    /**
     * int sockfd1, new_fd1;  // listen on sock_fd, new connection on new_fd
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

    */

    sin_size1 = sizeof their_addr1;
		new_fd1 = accept(sockfd1, (struct sockaddr *)&their_addr1, &sin_size1);
		if (new_fd1 == -1) {
			perror("accept");
			//continue;
		}

		inet_ntop(their_addr1.ss_family,
			get_in_addr((struct sockaddr *)&their_addr1),
			s1, INET6_ADDRSTRLEN);
        // replace sizeof s1 with INET6_ADDRSTRLEN
		//printf("server: got connection from %s\n", s1);

		if (!fork()) { // this is the child process
			close(sockfd1); // child doesn't need the listener
            
            // START TALKING HERE

            // WAIT FOR CLIENT COMMAND
            if ((numbytes1 = recv(new_fd1, buf1, MAXDATASIZE-1, 0)) == -1) 
            {
                perror("recv");
	        }
            buf1[numbytes1] = '\0'; // ending null char
            //printf("serverM: received '%s'\n",buf1);

            // CHECK WALLET OPERATIONS
            if (buf1[0] == 'C' && buf1[1] == 'W')
            {
                int totalBalance = 0;
                string username(buf1);
                username = username.substr(3, string::npos);
                printf("The main server received input='%s' from the client using TCP over port %s.\n", username.c_str(), portNum.c_str());

                // TALK TO SERVER A
                // send req to server A, put buf1 here because want to relay message from CA
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
                {
                    perror("server M to serverA: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                printf("The main server sent a request to server A.\n");

                // receive req info
                // always put following line before recvfrom
                servAaddr_len = sizeof servAaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                //printf("serverM: received '%s'\n", buf);
                printf("The main server received transactions from Server A using UDP over port %s.\n", PORTSA);
                string balanceA(buf);
                string statusA = balanceA.substr(0, 1);
                balanceA = balanceA.substr(2, string::npos);
                totalBalance = totalBalance + stoint(balanceA);

                // TALK TO SERVER B
                // send req to server B, put buf1 here because want to relay message from CA
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
                {
                    perror("server M to serverB: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                printf("The main server sent a request to server B.\n");

                // receive req info
                // always put following line before recvfrom
                servBaddr_len = sizeof servBaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                //printf("serverM: received '%s'\n", buf);
                printf("The main server received transactions from Server B using UDP over port %s.\n", PORTSB);
                string balanceB(buf);
                string statusB = balanceB.substr(0, 1);
                balanceB = balanceB.substr(2, string::npos);
                totalBalance = totalBalance + stoint(balanceB);

                // TALK TO SERVER C
                // send req to server C, put buf1 here because want to relay message from CA
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
                {
                    perror("server M to serverC: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                printf("The main server sent a request to server C.\n");

                // receive req info
                // always put following line before recvfrom
                servCaddr_len = sizeof servCaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                //printf("serverM: received '%s'\n", buf);
                printf("The main server received transactions from Server C using UDP over port %s.\n", PORTSC);
                string balanceC(buf);
                string statusC = balanceC.substr(0, 1);
                balanceC = balanceC.substr(2, string::npos);
                totalBalance = totalBalance + stoint(balanceC);

                // add initial balance
                totalBalance = 1000 + totalBalance;

                // cannot find person in network
                if (statusA.compare("F") == 0 && statusB.compare("F") == 0 && statusC.compare("F")== 0)
                {
                    if (send(new_fd1, "F", strlen("F"), 0) == -1)
                    {
                        perror("send");
                    }
                }
                else
                {
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, to_string(totalBalance, 0).c_str(), strlen(to_string(totalBalance, 0).c_str()), 0) == -1)
                    {
                        perror("send");
                    }
                }

                //printf("serverM: send '%s'\n", to_string(totalBalance).c_str());
                printf("The main server sent the current balance to client %s.\n", client.c_str());
            }
            // TXCOINS
            else if (buf1[0] == 'T' && buf1[1] == 'C')
            {
                string username1(buf1);
                // get rid of the code at the front
                username1 = username1.substr(3, string::npos);
                // save username2
                string username2 = username1;
                // amount
                int amt = stoint(username1.substr(username1.find_last_of(" ") + 1, string::npos));
                username1 = username1.substr(0, username1.find_first_of(" "));
                username2 = username2.substr(username2.find_first_of(" ") + 1, string::npos);
                username2 = username2.substr(0, username2.find_first_of(" "));

                printf("The main server received from %s to transfer %s coins to %s using TCP over port %s.\n", username1.c_str(), to_string(amt, 0).c_str(), username2.c_str(), portNum.c_str());

                string results1 = checkWallet(sockfd, username1, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len);
                string results2 = checkWallet(sockfd, username2, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len);
                int results1Balance = stoint(results1.substr(2, string::npos));

                // BOTH NOT IN NETWORK
                if (results1.substr(0, 1).compare("F") == 0 && results2.substr(0,1).compare("F") == 0)
                {
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, "TC BN", strlen("TC BN"), 0) == -1)
                    {
                        perror("send");
                    }
                }
                // USERNAME 1 NOT IN NETWORK
                else if (results1.substr(0, 1).compare("F") == 0)
                {
                    string msgTCON = "TC ON " + username1;
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, msgTCON.c_str(), strlen(msgTCON.c_str()), 0) == -1)
                    {
                        perror("send");
                    }
                }
                // USERNAME 2 NOT IN NETWORK
                else if (results2.substr(0, 1).compare("F") == 0)
                {
                    string msgTCON = "TC ON " + username2;
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, msgTCON.c_str(), strlen(msgTCON.c_str()), 0) == -1)
                    {
                        perror("send");
                    }
                }
                // INSUFFICIENT BALANCE
                else if (results1Balance < amt)
                {
                    string msgTCIB = "TC IB " + to_string(results1Balance, 0);
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, msgTCIB.c_str(), strlen(msgTCIB.c_str()), 0) == -1)
                    {
                        perror("send");
                    }
                }
                // SUCCESSFUL TRANSACTION
                else
                {
                    results1Balance = results1Balance - amt;
                    int maxSequence = findSequenceNumber(sockfd, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len) + 1;
                    string log = "TC " + to_string(maxSequence, 0) + " " + username1 + " " + username2 + " " + to_string(amt, 0);
                    //printf("THE CURRENT SEQ NUM IS %s\n", to_string(maxSequence).c_str());
                    
                    // https://stackoverflow.com/questions/9711076/why-does-rand-always-return-the-same-value
                    srand (time(NULL));
                    int randServer = rand() % 3;
                    //printf("THE SERVER NUMBER IS %s\n", to_string(randServer).c_str());

                    // Server A
                    if (randServer == 0)
                    {
                        // A
                        if ((numbytes = sendto(sockfd, log.c_str(), strlen(log.c_str()), 0,
                        (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
                        {
                            perror("server M to serverA: sendto");
                            exit(1);
                        }
                        // always put following line before recvfrom
                        servAaddr_len = sizeof servAaddr;
                        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                            (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
                        {
                            perror("recvfrom");
                            exit(1);
                        }
                        buf[numbytes] = '\0';
                    }
                    // Server B
                    else if (randServer == 1)
                    {
                        // B
                        if ((numbytes = sendto(sockfd, log.c_str(), strlen(log.c_str()), 0,
                        (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
                        {
                            perror("server M to serverB: sendto");
                            exit(1);
                        }
                        // always put following line before recvfrom
                        servBaddr_len = sizeof servBaddr;
                        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                            (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
                        {
                            perror("recvfrom");
                            exit(1);
                        }
                        buf[numbytes] = '\0';
                    }
                    // Server C
                    else if (randServer == 2)
                    {
                        // C
                        if ((numbytes = sendto(sockfd, log.c_str(), strlen(log.c_str()), 0,
                        (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
                        {
                            perror("server M to serverC: sendto");
                            exit(1);
                        }
                        // receive req info
                        // always put following line before recvfrom
                        servCaddr_len = sizeof servCaddr;
                        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                            (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
                        {
                            perror("recvfrom");
                            exit(1);
                        }
                        buf[numbytes] = '\0';
                    }

                    string msgTCSC = "TC SC " + to_string(results1Balance, 0);
                    // SEND REQUESTED INFO MESSAGE TO CLIENT
                    if (send(new_fd1, msgTCSC.c_str(), strlen(msgTCSC.c_str()), 0) == -1)
                    {
                        perror("send");
                    }
                    printf("The main server sent the result of the transaction to client %s.\n", client.c_str());
                }
            }
            // TXLIST
            else if (buf1[0] == 'T' && buf1[1] == 'L')
            {
                printf("A sorted list request from client %s has been received.\n", client.c_str());
                string txList;
                // TALK TO SERVER A
                // send req to server A, put buf1 here because want to relay message from CA
                if ((numbytes = sendto(sockfd, "TL", strlen("TL"), 0,
                        (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
                {
                    perror("server M to serverA: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                // receive req info from A
                // always put following line before recvfrom
                servAaddr_len = sizeof servAaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string temp1(buf);
                txList = txList + temp1;
                //printf("serverM: received '%s'\n", buf);

                // TALK TO SERVER B
                if ((numbytes = sendto(sockfd, "TL", strlen("TL"), 0,
                        (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
                {
                    perror("server M to serverB: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                // receive req info from B
                // always put following line before recvfrom
                servBaddr_len = sizeof servBaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string temp2(buf);
                txList = txList + temp2;
                //printf("serverM: received '%s'\n", buf);

                // TALK TO SERVER C
                // send req to server C, put buf1 here because want to relay message from CA
                if ((numbytes = sendto(sockfd, "TL", strlen("TL"), 0,
                        (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
                {
                    perror("server M to serverC: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");
                // receive req info from C
                // always put following line before recvfrom
                servCaddr_len = sizeof servCaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string temp3(buf);
                txList = txList + temp3;
                //printf("serverM: received '%s'\n", buf);
                int size = findSequenceNumber(sockfd, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len);
                writeTXLIST(txList, size);
                // printf("CONTENT OF ALICHAIN.TXT IS '%s'.\n", txList.c_str());
                // printf("SIZE OF ALICHAIN.TXT IS '%s'.\n", to_string(size).c_str());

                // SEND REQUESTED INFO MESSAGE TO CLIENT
                // use buf1 here because it is currently TL code
                if (send(new_fd1, "done writing", strlen("done writing"), 0) == -1)
                {
                    perror("send");
                }
                //printf("serverM: send '%s'\n", "done writing");
                printf("The sorted file is up and ready.\n");
            }
            // STATS code ST
            else if (buf1[0] == 'S' && buf1[1] == 'T')
            {
                string listUnranked = "";
                string listRanked = "";
                string username(buf);
                username = username.substr(3, string::npos);

                // TALK TO SERVER A
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servAaddr, sizeof(servAaddr))) == -1) 
                {
                    perror("server M to serverA: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

                // receive req info from A and store in buf
                // always put following line before recvfrom
                servAaddr_len = sizeof servAaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string listA(buf);
                listUnranked = listUnranked + listA;
                //printf("serverM: received '%s'\n", buf);

                // TALK TO SERVER B
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servBaddr, sizeof(servBaddr))) == -1) 
                {
                    perror("server M to serverB: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

                // receive req info from B and store in buf
                // always put following line before recvfrom
                servBaddr_len = sizeof servBaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string listB(buf);
                listUnranked = listUnranked + listB;
                //printf("serverM: received '%s'\n", buf);
                
                // TALK TO SERVER C
                if ((numbytes = sendto(sockfd, buf1, strlen(buf1), 0,
                        (struct sockaddr *) &servCaddr, sizeof(servCaddr))) == -1) 
                {
                    perror("server M to serverC: sendto");
                    exit(1);
                }
                //printf("server M: sent %d bytes to %s\n", numbytes, "127.0.0.1");

                // receive req info from C and store in buf
                // always put following line before recvfrom
                servCaddr_len = sizeof servCaddr;
                if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
                    (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                string listC(buf);
                listUnranked = listUnranked + listC;
                //printf("serverM: received '%s'\n", buf);

                string finalRanking = getStats(listUnranked);

                // SEND REQUESTED INFO MESSAGE TO CLIENT
                if (send(new_fd1, finalRanking.c_str(), strlen(finalRanking.c_str()), 0) == -1)
                {
                    perror("send");
                }
                printf("The main server sent the statistics to client %s.\n", client.c_str());
            }
            // NO VALID COMMAND
            else
            {
                //printf("Not a valid command.\n");
            }

            // DONE TALKING HERE
			close(new_fd1);
			exit(0);
		}
		close(new_fd1);  // parent doesn't need this
}

int main(void)
{   
    // CREATE SERVER M UDP SOCKET
    int sockfd; // main serverM socket
    int numbytes; // number of bytes in message
    char buf[MAXDATASIZE]; // buffer to store recvfrom messages

    // create serverM UDP socket
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    printf("The main server is up and running.\n");

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

    // SERVER B INFO
    struct sockaddr_in servBaddr;
    socklen_t servBaddr_len;
    servBaddr.sin_family = AF_INET;
	servBaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	servBaddr.sin_port=htons(22421); //source port for outgoing packets

    // SERVER C INFO
    struct sockaddr_in servCaddr;
    socklen_t servCaddr_len;
    servCaddr.sin_family = AF_INET;
	servCaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	servCaddr.sin_port=htons(23421); //source port for outgoing packets

    // bind serverM socket to serverM addr info
    if(bind(sockfd, (struct sockaddr*)&servMaddr, sizeof(servMaddr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    //printf("serverM: waiting to recvfrom...\n");
    // close(sockfd); keep on

    // DONE CREATING UDP SOCKET

    // WAIT FOR SERVERA TO SEND MESSAGE (JUST AS TEST)
    // always put following line before recvfrom
    servAaddr_len = sizeof servAaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servAaddr, &servAaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverM: received '%s'\n", buf);

    // WAIT FOR SERVERB TO SEND MESSAGE (JUST AS TEST)
    // always put following line before recvfrom
    servBaddr_len = sizeof servBaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servBaddr, &servBaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverM: received '%s'\n", buf);

    // WAIT FOR SERVERC TO SEND MESSAGE (JUST AS TEST)
    // always put following line before recvfrom
    servCaddr_len = sizeof servCaddr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *) &servCaddr, &servCaddr_len)) == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //printf("serverC: received '%s'\n", buf);

    // END UDP SOCKET

    // CREATE TCP SOCKET 1
	int sockfd1 = 0;
    int new_fd1 = 0;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints1, *servinfo1, *p1;
	struct sockaddr_storage their_addr1; // connector's address information
	socklen_t sin_size1 = 0;
	struct sigaction sa1;
	int yes1 = 1;
	char s1[INET6_ADDRSTRLEN];
	int rv1 = 0;
    // for receiving messages from TCP sockets
    int numbytes1 = 0; // check message length
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
	int sockfd2 = 0;
    int new_fd2 = 0;  // listen on sock_fd2, new connection on new_fd2
	struct addrinfo hints2, *servinfo2, *p2;
	struct sockaddr_storage their_addr2; // connector's address information
	socklen_t sin_size2 = 0;
	struct sigaction sa2;
	int yes2 = 1;
	char s2[INET6_ADDRSTRLEN];
	int rv2 = 0;
    // for receiving messages from TCP sockets
    int numbytes2 = 0; // check message length
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
    
	//printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
        // LISTEN FOR CLIENT A
        serverMOperations(sockfd, sockfd1, new_fd1, numbytes, numbytes1, s1, buf, buf1, sin_size1, their_addr1, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len, "A", PORTCA);
        // END TCP ACCEPT FOR CLIENT A
        
        // LISTEN FOR CLIENT B
        serverMOperations(sockfd, sockfd2, new_fd2, numbytes, numbytes2, s2, buf, buf2, sin_size2, their_addr2, servAaddr, servAaddr_len, servBaddr, servBaddr_len, servCaddr, servCaddr_len, "B", PORTCB);
        // END TCP ACCEPT TO CLIENT B  
	}

	return 0;
}
