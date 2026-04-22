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
using namespace std::chrono;
using namespace std;

int height, width;
struct image_t *input_image;
struct image_t *sharpened_image;
int bb = 2;
float alpha = 1;
bool debug = false;
struct pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    int x, y;
};

void S1_smoothen(struct image_t *input_image, int read_fd, int write_fd)
{
    // TODO
    // remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.

    for (int i = 0; i < input_image->height; i++)
    {
        pixel *row = new pixel[input_image->width];
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
        size_t bw = write(write_fd, row, input_image->width * sizeof(pixel));
        delete row;
        if (bw < input_image->width * sizeof(pixel))
        {
            cout << "write error\n";
        }
        if (debug)
            cout << "S1 on Processed row (" << i << ")\n";
    }
    // printf("%d", si->width);
}

void S2_find_details(struct image_t *input_image, int read_fd, int write_fd)
{
    int rows_read = 0;

    // TODO
    while (1)
    {
        pixel *row = new pixel[input_image->width];
        pixel *out_row = new pixel[input_image->width];
        char *readp = (char *)row;

        size_t bytes_read = 0;
        while (bytes_read < sizeof(pixel) * input_image->width)
        {
            ssize_t r = read(read_fd, readp + bytes_read, sizeof(pixel) * input_image->width - bytes_read);
            if (r <= 0)
            {
                // cout << "S2 read errorpopo\n";
                continue;
            }
            bytes_read += r;
        }

        // ssize_t bytes_read = read(read_fd, readp, sizeof(pixel) * input_image->width);
        if (bytes_read == 0)
            continue;
        if (bytes_read < input_image->width * sizeof(pixel))
        {
            cout << "S2 read error\n";
        }
        if (debug)
            cout << "S2:" << row[0].x << " " << row[0].y << " " << (int)row[0].r << " " << (int)row[0].g << " " << (int)row[0].b << endl;
        rows_read++;

        if (rows_read > input_image->height)
            break;

        for (int i = 0; i < input_image->width; i++)
        {
            pixel p = row[i];
            int find_details_value_r = input_image->image_pixels[p.y][p.x][0] - p.r;
            int find_details_value_g = input_image->image_pixels[p.y][p.x][1] - p.g;
            int find_details_value_b = input_image->image_pixels[p.y][p.x][2] - p.b;

            // pixel out_p;
            out_row[i].x = p.x;
            out_row[i].y = p.y;
            out_row[i].r = (find_details_value_r > 0) ? find_details_value_r : 0;
            out_row[i].g = (find_details_value_g > 0) ? find_details_value_g : 0;
            out_row[i].b = (find_details_value_b > 0) ? find_details_value_b : 0;
        }

        // int find_details_value_r = input_image->image_pixels[p.y][p.x][0] - p.r;
        // int find_details_value_g = input_image->image_pixels[p.y][p.x][1] - p.g;
        // int find_details_value_b = input_image->image_pixels[p.y][p.x][2] - p.b;

        // pixel out_p;
        // out_p.x = p.x;
        // out_p.y = p.y;
        // out_p.r = (find_details_value_r > 0) ? find_details_value_r : 0;
        // out_p.g = (find_details_value_g > 0) ? find_details_value_g : 0;
        // out_p.b = (find_details_value_b > 0) ? find_details_value_b : 0;

        size_t bw = write(write_fd, out_row, sizeof(pixel) * input_image->width);
        delete row, out_row;
        if (bw < input_image->width * sizeof(pixel))
        {
            cout << "write error\n";
        }
        if (debug)
            cout << "S2 on Processed row (" << rows_read << ")\n";
        if (rows_read >= input_image->height)
            break;
    }
}

void S3_sharpen(struct image_t *input_image, int read_fd)
{
    // TODO
    int rows_read = 0;

    while (1)
    {
        pixel *row = new pixel[input_image->width];
        char *readp = (char *)row;
        pixel p;
        size_t bytes_read = 0;
        while (bytes_read < sizeof(pixel) * input_image->width)
        {
            ssize_t r = read(read_fd, readp + bytes_read, sizeof(pixel) * input_image->width - bytes_read);
            if (r <= 0)
            {
                cout << "S3 read error\n";
                continue;
            }
            bytes_read += r;
        }
        if (bytes_read == 0)
            continue;
        if (bytes_read < input_image->width * sizeof(pixel))
        {
            cout << "s3 read error\n";
        }
        rows_read++;
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
        if (rows_read >= input_image->height)
            break;
        delete row;
    }
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

    // auto start = high_resolution_clock::now();
    input_image = read_ppm_file(argv[1]);
    height = input_image->height;
    width = input_image->width;
    int S1_S2_pipe[2];
    int S2_S3_pipe[2];
    pipe(S1_S2_pipe);
    pipe(S2_S3_pipe);
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
    auto start = high_resolution_clock::now();
    pid_t S1_pid = fork();

    if (S1_pid == 0)
    {
        close(S1_S2_pipe[0]);
        cout << "S1 process:" << getpid() << endl;

        for (int i = 0; i < noi; i++)
        {
            S1_smoothen(input_image, -1, S1_S2_pipe[1]);
            cout << "Process S1 completed iteration " << i + 1 << "\n";
        }
        close(S1_S2_pipe[1]);
        exit(0);
    }
    else
    {
        pid_t S2_pid = fork();
        if (S2_pid == 0)
        {
            close(S1_S2_pipe[1]);
            close(S2_S3_pipe[0]);
            cout << "S2 process:" << getpid() << endl;

            for (int i = 0; i < noi; i++)
            {
                S2_find_details(input_image, S1_S2_pipe[0], S2_S3_pipe[1]);
                cout << "Process S2 completed iteration " << i + 1 << "\n";
            }
            close(S1_S2_pipe[0]);
            close(S2_S3_pipe[1]);
            exit(0);
        }
        else
        {
            close(S1_S2_pipe[0]);
            close(S1_S2_pipe[1]);
            close(S2_S3_pipe[1]);
            cout << "S3 process:" << getpid() << endl;

            for (int i = 0; i < noi; i++)
            {
                S3_sharpen(input_image, S2_S3_pipe[0]);
                cout << "Process S3 completed iteration " << i + 1 << "\n";
            }
            auto stop = chrono::high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            cout << "Time for " << noi << "  iterations:" << duration.count() << "milliseconds" << endl;
            write_ppm_file(argv[2], sharpened_image);
            close(S2_S3_pipe[0]);
            // waitpid(S2_pid, NULL, 0);
            // waitpid(S1_pid, NULL, 0);
            exit(0);
        }
    }
}