#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "libppm.h"
#include <cstdint>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;


// S3 server
#define PORT "9003"

struct __attribute__((packed)) pixel {
    uint8_t r, g, b;
    int x, y;
};

float alpha = 1;

void S3_server_sharpen(struct image_t *input_image, struct image_t *output_image, int client_fd) {
    int width=input_image->width;
    int height=input_image->height;
    pixel *row=new pixel[width];
    while (true)
    {
        int row_index;
        char *buffer=(char*)malloc(sizeof(int) + sizeof(pixel)*width);
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(int) + sizeof(pixel)*width, 0);
        if (bytes_read <= 0)
        {
            free(buffer);
            break;
        }

        memcpy(&row_index, buffer, sizeof(int));
        memcpy(row, buffer + sizeof(int), sizeof(pixel)*width);
        free(buffer);

        for (int i=0;i<width;i++)
        {
            pixel p = row[i];
            int r=input_image->image_pixels[p.y][p.x][0] + static_cast<int>(alpha * p.r);
            int g=input_image->image_pixels[p.y][p.x][1] + static_cast<int>(alpha * p.g);
            int b=input_image->image_pixels[p.y][p.x][2] + static_cast<int>(alpha * p.b);
            output_image->image_pixels[p.y][p.x][0] = min(255, r);
            output_image->image_pixels[p.y][p.x][1] = min(255, g);
            output_image->image_pixels[p.y][p.x][2] = min(255, b);
        }
    }
    delete[] row;
}

int main(int argc, char **argv) {
    if (argc!=3) {
        cerr<<"usage: ./server <input-image> <output-image>"<<endl;
        exit(1);
    }

    struct image_t *input_image = read_ppm_file(argv[1]);

    struct image_t *sharpened_image = new struct image_t;
    sharpened_image->width = width;
    sharpened_image->height = height;
    sharpened_image->image_pixels = new uint8_t **[height];
    for (int i=0;i<height;i++)
    {
        sharpened_image->image_pixels[i] = new uint8_t *[width];
        for (int j=0;j<width;j++)
            sharpened_image->image_pixels[i][j] = new uint8_t[3];
    }

    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(nullptr, PORT, &hints, &res);
    if (status != 0) {
        cerr<<"getaddrinfo error: "<<gai_strerror(status)<<endl;
        return 1;
    }

    int server_fd=socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    freeaddrinfo(res);

    if (listen(server_fd, 1) == -1) {
        perror("listen");
        return 1;
    }

    cout << "Server listening on port " << PORT << "...\n";

    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1) {
        perror("accept");
        return 1;
    }

    cout << "Client connected, starting S3...\n";
    auto start = high_resolution_clock::now();

    S3_server_sharpen(input_image, sharpened_image, client_fd);

    auto end = high_resolution_clock::now();
    cout << "Processing done in " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    write_ppm_file(argv[2], sharpened_image);

    close(client_fd);
    close(server_fd);
    free_image(input_image);
    free_image(sharpened_image);
    return 0;
}
