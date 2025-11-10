#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>
#include <cstring>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "9003" // service port number as a string

using namespace std::chrono;
using namespace std;

int height, width;
struct image_t *input_image;
struct image_t *sharpened_image;
int bb = 2;
float alpha = 1;
bool debug = true;

struct __attribute__((packed)) pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    int x, y;
};

int rr = 0;
char *ptr[sizeof(int) + sizeof(pixel) * 10000]; // assuming max width 10000

ssize_t send_all(int sock, const void *buf, size_t len)
{
    size_t total = 0;
    const char *p = (const char *)buf;
    while (total < len)
    {
        ssize_t sent = send(sock, p + total, len - total, 0);
        if (sent <= 0)
            return sent;
        total += sent;
    }
    return total;
}

ssize_t recv_all(int sock, void *buf, size_t len)
{
    size_t total = 0;
    char *p = (char *)buf;
    while (total < len)
    {
        ssize_t recvd = recv(sock, p + total, len - total, 0);
        if (recvd <= 0)
            return recvd;
        total += recvd;
    }
    return total;
}

void S1_smoothen(struct image_t *input_image, int client_fd)
{
    // TODO
    // remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.
    pixel *row = new pixel[input_image->width];

    for (int i = 0; i < input_image->height; i++)
    {

        for (int j = 0; j < input_image->width; j++)
        {
            int r = 0, g = 0, b = 0;
            for (int y = -bb; y <= bb; y++)
            {
                for (int x = -bb; x <= bb; x++)
                {
                    if (i + y < 0)
                        continue;
                    if (i + y >= input_image->height)
                        continue;
                    if (j + x < 0)
                        continue;
                    if (j + x >= input_image->width)
                        continue;
                    r += input_image->image_pixels[i + y][j + x][0];
                    g += input_image->image_pixels[i + y][j + x][1];
                    b += input_image->image_pixels[i + y][j + x][2];
                }
            }

            row[j].r = r / ((2 * bb + 1) * (2 * bb + 1));
            row[j].g = g / ((2 * bb + 1) * (2 * bb + 1));
            row[j].b = b / ((2 * bb + 1) * (2 * bb + 1));
            row[j].x = j;
            row[j].y = i;
            // write(write_fd, &p, sizeof(pixel));
        }

        // sem_wait(S2_S1_sem);
        // memcpy(S1_S2_ptr, &i, sizeof(int));
        // memcpy(((char *)S1_S2_ptr) + sizeof(int), row, sizeof(pixel) * input_image->width);
        // sem_post(S1_S2_sem);
        char *p = (char *)malloc(sizeof(int) + sizeof(pixel) * input_image->width);
        memset(p, 0, sizeof(int) + sizeof(pixel) * input_image->width);
        memcpy(p, &i, sizeof(int));
        memcpy(p + sizeof(int), row, sizeof(pixel) * input_image->width);
        send_all(client_fd, (char *)p, sizeof(int) + sizeof(pixel) * input_image->width);
        while (1)
        {
            int br = recv_all(client_fd, (char *)&rr, sizeof(int));
            if (br < 0)
            {
                cout << "Client disconnected.\n";
                exit(1);
            }
            if (br == 0)
            {
                if (debug)
                    cout << "Nothing to recv/n";
                continue;
            }
            if (br > 0)
                break;
        }
        free(p);
        // recv(client_fd, (char *)&rr, sizeof(int), 0);

        if (debug)
            cout << "S1 on Processed row (" << i << ")\n";
        // delete[] row;
        // while (1)
        // {
        //     sem_wait(S1_S2_sem);
        //     int x = *((int *)S1_S2_ptr);
        //     // cout << "S1 sees row index in shm: " << x << endl;
        //     sem_post(S1_S2_sem);
        //     if (x == -1)
        //         break;
        // }
    }
    delete[] row;
    // printf("%d", si->width);
}

void free_image(image_t *img)
{
    for (int i = 0; i < img->height; ++i)
    {
        for (int j = 0; j < img->width; ++j)
            delete[] img->image_pixels[i][j];
        delete[] img->image_pixels[i];
    }
    delete[] img->image_pixels;
    delete img;
}

int main(int argc, char **argv)
{
    int noi = 1000;

    if (argc != 4)
    {

        cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image> <number of iterations>\n\n";
        exit(0);
    }
    noi = atoi(argv[3]);

    input_image = read_ppm_file(argv[1]);
    height = input_image->height;
    width = input_image->width;

    sharpened_image = new struct image_t;
    sharpened_image->width = input_image->width;
    sharpened_image->height = input_image->height;
    sharpened_image->image_pixels = new uint8_t **[sharpened_image->height];
    for (int i = 0; i < sharpened_image->height; i++)
    {
        sharpened_image->image_pixels[i] = new uint8_t *[sharpened_image->width];
        for (int j = 0; j < sharpened_image->width; j++)
            sharpened_image->image_pixels[i][j] = new uint8_t[3];
    }

    cout << "Height:" << height << endl;

    int status;
    struct addrinfo hints{}, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(nullptr, PORT, &hints, &res);
    if (status != 0)
    {
        cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return 1;
    }

    int server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    freeaddrinfo(res);

    if (listen(server_fd, 1) == -1)
    {
        perror("listen");
        return 1;
    }

    cout << "Server listening on port " << PORT << "...\n";

    // Accept client
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1)
    {
        perror("accept");
        return 1;
    }

    cout << "Client connected!\n";

    auto start = chrono::high_resolution_clock::now();

    cout << "S1 process:" << getpid() << endl;

    for (int i = 0; i < noi; i++)
    {
        S1_smoothen(input_image, client_fd);
        cout << "Process S1 completed iteration " << i + 1 << "\n";
    }

    exit(0);
}