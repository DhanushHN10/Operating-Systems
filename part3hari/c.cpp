#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "9003"

int main()
{
    int status;
    struct addrinfo hints{}, *res;

    // Input server address
    std::string server_ip;
    std::cout << "Enter server IP: ";
    // std::cin >> server_ip;
    std::cin.ignore();

    // Prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Get address info for the server
    server_ip = "localhost";
    status = getaddrinfo(server_ip.c_str(), PORT, &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return 1;
    }

    // Create socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    // Connect to server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    freeaddrinfo(res);

    std::cout << "Connected to server.\n";

    char buffer[1024];
    while (true)
    {
        // std::string msg;
        // std::cout << "You: ";
        // std::getline(std::cin, msg);
        // send(sockfd, msg.c_str(), msg.size(), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            std::cout << "Server disconnected.\n";
            break;
        }
        int a = *((int *)buffer);

        std::cout << "Server: " << a << "\n";
        send(sockfd, (char *)&a, sizeof(int), 0);
    }

    close(sockfd);
    return 0;
}
