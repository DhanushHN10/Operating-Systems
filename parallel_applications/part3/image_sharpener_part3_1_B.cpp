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
bool debug = false;
struct __attribute__((packed)) pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    int x, y;
};

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

void S3_sharpen(struct image_t *input_image, int sockfd)
{
    // TODO
    int rows_read = 0;
    pixel *row = new pixel[input_image->width];

    while (1)
    {

        char *readp = (char *)row;
        int row_index;
        char *p = (char *)malloc(sizeof(int) + sizeof(pixel) * input_image->width);
        memset(p, 0, sizeof(int) + sizeof(pixel) * input_image->width);
        size_t bytes_read = recv_all(sockfd, (char *)p, sizeof(int) + sizeof(pixel) * input_image->width);
        if (bytes_read <= 0)
        {
            cout << "Client disconnected.\n";
            exit(1);
        }
        memcpy(&row_index, p, sizeof(int));
        if (row_index != rows_read)
        {
            if (debug)
                cout << "S2 row index duplicate got" << row_index << "expected " << rows_read << endl;
            continue;
        }

        memcpy(row, p + sizeof(int), sizeof(pixel) * input_image->width);
        rows_read++;
        send_all(sockfd, (char *)&row_index, sizeof(int));
        free(p);

        // ssize_t bytes_read = read(read_fd, readp, sizeof(pixel) * input_image->width);
        if (debug)
            cout << "S3 got row index " << row_index << endl;
        // sem_post(S3_S2_sem);

        // if (row_index != rows_read)
        // {
        //     if (debug)
        //         cout << "S3 row index duplicate" << rows_read << endl;
        //     // sem_post(S2_S3_sem);
        //     continue;
        // }
        // rows_read++;
        // sem_wait(S2_S3_sem);
        if (debug)
            cout << "S3:" << row[0].x << " " << row[0].y << " " << (int)row[0].r << " " << (int)row[0].g << " " << (int)row[0].b << endl;
        if (rows_read > input_image->width * input_image->height)
            break;
        if (debug)
            cout << "S3 on Received Pixel (" << row[0].x << "," << row[0].y << ")" << "Pixels Read: " << rows_read << endl;

        for (int i = 0; i < input_image->width; i++)
        {
            pixel p = row[i];
            int r = input_image->image_pixels[p.y][p.x][0] + static_cast<int>(alpha * p.r);
            int g = input_image->image_pixels[p.y][p.x][1] + static_cast<int>(alpha * p.g);
            int b = input_image->image_pixels[p.y][p.x][2] + static_cast<int>(alpha * p.b);
            if (r > 255)
                sharpened_image->image_pixels[p.y][p.x][0] = 255;
            else
                sharpened_image->image_pixels[p.y][p.x][0] = r;
            if (g > 255)
                sharpened_image->image_pixels[p.y][p.x][1] = 255;
            else
                sharpened_image->image_pixels[p.y][p.x][1] = g;
            if (b > 255)
                sharpened_image->image_pixels[p.y][p.x][2] = 255;
            else
                sharpened_image->image_pixels[p.y][p.x][2] = b;
        }

        if (rows_read >= input_image->height)
            break;
        // delete[] row;
    }
    delete[] row;
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

        cout << "usage: ./a.out <path-to-input-image> <path-to-transformed-image> <number of iterations>\n\n";
        exit(0);
    }
    noi = atoi(argv[3]);

    input_image = read_ppm_file(argv[1]);

    // sem_post(S2_S1_sem);

    // if (S1_S2_sem <= 0 || S1_S2_sem == SEM_FAILED || S2_S3_sem <= 0 || S2_S3_sem == SEM_FAILED)
    // {
    //     perror("semaphore open failed");
    //     return 1;
    // }

    int status;
    struct addrinfo hints{}, *res;

    std::string server_ip;
    std::cout << "Enter server IP: ";
    std::cin >> server_ip;
    std::cin.ignore();

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // server_ip = "localhost";
    status = getaddrinfo(server_ip.c_str(), PORT, &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return 1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    freeaddrinfo(res);

    std::cout << "Connected to server.\n";
    int *size = new int[2];
    ssize_t br = recv_all(sockfd, (char *)size, sizeof(int) * 2);
    if (br <= 0)
    {
        cout << "Client disconnected.\n";
        exit(1);
    }
    width = size[0];
    height = size[1];

    sharpened_image = new struct image_t;
    sharpened_image->width = width;
    sharpened_image->height = height;
    sharpened_image->image_pixels = new uint8_t **[sharpened_image->height];
    cout << "Height:" << height << endl;
    for (int i = 0; i < sharpened_image->height; i++)
    {
        sharpened_image->image_pixels[i] = new uint8_t *[sharpened_image->width];
        for (int j = 0; j < sharpened_image->width; j++)
            sharpened_image->image_pixels[i][j] = new uint8_t[3];
    }

    auto start = chrono::high_resolution_clock::now();

    cout << "S3 process:" << getpid() << endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(2));

    for (int i = 0; i < noi; i++)
    {
        S3_sharpen(input_image, sockfd);
        cout
            << "Process S3 completed iteration " << i + 1 << "\n";
    }
    auto stop = chrono::high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Time for " << noi << "  iterations:" << duration.count() << "milliseconds" << endl;

    write_ppm_file(argv[2], sharpened_image);

    exit(0);
}