using namespace std;

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    // int sockfd = socket(domain, type, protocol)
    int socketM = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketM > 0)
    {
        cout << "The main server is up and running." << endl;
    }
    else
    {
        cout << "The main server could not run." << endl;
    }

    // int bind (int socket, local address, address length);
    //int bindSocketM = bind(socketM, SOCK_DGRAM, 0);

    if (socketM > 0)
    {
        cout << "The main server is up and running." << endl;
    }
    else
    {
        cout << "The main server could not run." << endl;
    }
}