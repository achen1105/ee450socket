#include <iostream>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

using namespace std;


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

    /**
    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                    const char *service,  // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);
    */
    getaddrinfo("127.0.0.1", "24421", &hints, &res);

    // make a socket:
    // int socket(int domain, int type, int protocol); 
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():
    // int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
}