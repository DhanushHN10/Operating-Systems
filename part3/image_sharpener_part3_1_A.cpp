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

#define PORT "9003"

struct __attribute__((packed)) pixel {
    uint8_t r, g, b;
    int x, y;
};

int bb = 2;
float alpha = 1;

void S1_S2(struct image_t *input_image, int webSocket)
{

     int height = input_image->height;
    int width = input_image->width;
    pixel *row = new pixel[width];
    pixel *out_row = new pixel[width];

    // S1 Smoothen

   

    for (int i=0; i<height;i++)
    {
        
        for (int j=0;j<width;j++)
        {
            int r = 0, g = 0, b = 0;
            for (int y=-bb;y<=bb;y++) {
                for (int x=-bb;x<=bb;x++) {
                    if (i+y<0 || i+y>=height ||
                        j+x<0 || j+x>=width)
                        continue;
                    r+=input_image->image_pixels[i + y][j + x][0];
                    g+=input_image->image_pixels[i + y][j + x][1];
                    b+=input_image->image_pixels[i + y][j + x][2];
                }
            }
            row[j].r = r / ((2 * bb + 1) * (2 * bb + 1));
            row[j].g = g / ((2 * bb + 1) * (2 * bb + 1));
            row[j].b = b / ((2 * bb + 1) * (2 * bb + 1));
            row[j].x = j;
            row[j].y = i;
        }

        //  S2 Find Details
        for (int j=0;j<width; j++) {
            int dr = input_image->image_pixels[i][j][0] - row[j].r;
            int dg = input_image->image_pixels[i][j][1] - row[j].g;
            int db = input_image->image_pixels[i][j][2] - row[j].b;
            out_row[j].r = max(0, dr);
            out_row[j].g = max(0, dg);
            out_row[j].b = max(0, db);
            out_row[j].x = row[j].x;
            out_row[j].y = row[j].y;
        }

        //  Passing data to S3 


        char *packet = (char*)malloc(sizeof(int) + sizeof(pixel) * width);
        memcpy(packet, &i, sizeof(int));
        memcpy(packet + sizeof(int), out_row, sizeof(pixel) * width);
        send(webSocket, packet, sizeof(int) + sizeof(pixel) * width, 0);
        free(packet);
    }

    delete[] row;
    delete[] out_row;
}

int main(int argc, char **argv) {
    if (argc!=5) {
        cerr << "usage: ./client <server-ip> <input-image> <output-image> <num-iterations>"<<endl;
        exit(1);
    }

    string server_ip = argv[1];
    string input_path = argv[2];
    int num_iterations = atoi(argv[4]);

    struct image_t *input_image=read_ppm_file(input_path.c_str());

    struct addrinfo hints{}, *res;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;

    int status = getaddrinfo(server_ip.c_str(), PORT, &hints, &res);
    if (status != 0) {
        cerr << "getaddrinfo error: " <<gai_strerror(status)<<endl;
        return 1;
    }

    int webSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (webSocket == -1) {
        perror("socket");
        return 1;
    }

    if (connect(webSocket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        close(webSocket);
        return 1;
    }

    freeaddrinfo(res);
    cout << "Connected to S3 server."<<endl;

    auto start = high_resolution_clock::now();
    for (int i=0;i<num_iterations;i++) {
        S1_S2(input_image, webSocket);
        cout<<"Completed iteration "<<i+1<< endl;
    }
    auto end = high_resolution_clock::now();
    cout << "Total time: "<<duration_cast<milliseconds>(end - start).count()<<" ms"<<endl;

    close(webSocket);
    free_image(input_image);
    return 0;
}
