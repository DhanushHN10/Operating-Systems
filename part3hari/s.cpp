#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "9002"
int main()
{
    int status;
    struct addrinfo hints{}, *res;

    // Set up hints for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // for binding

    // Get address info for server
    status = getaddrinfo(nullptr, PORT, &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return 1;
    }

    // Create socket
    int server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    // Bind socket
    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    freeaddrinfo(res); // done with res

    // Listen for connections
    if (listen(server_fd, 1) == -1)
    {
        perror("listen");
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    // Accept client
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1)
    {
        perror("accept");
        return 1;
    }

    std::cout << "Client connected!\n";

    // Communication loop
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::cout << "Client: " << buffer << "\n";

        std::string reply;
        std::cout << "You: ";
        std::getline(std::cin, reply);

        send(client_fd, reply.c_str(), reply.size(), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
