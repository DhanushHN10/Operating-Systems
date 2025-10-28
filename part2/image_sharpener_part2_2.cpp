#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <unistd.h>
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
    struct image_t *si = new struct image_t;
    si->width = input_image->width;
    si->height = input_image->height;
    si->image_pixels = new uint8_t **[si->height];
    pixel *row = new pixel[si->width];
    for (int i = 0; i < si->height; i++)
    {
        si->image_pixels[i] = new uint8_t *[si->width];
        for (int j = 0; j < si->width; j++)
            si->image_pixels[i][j] = new uint8_t[3];
    }
    for (int i = 0; i < si->height; i++)
    {
        for (int j = 0; j < si->width; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                si->image_pixels[i][j][k] = input_image->image_pixels[i][j][k];
            }
        }
    }

    for (int i = 0; i < si->height; i++)
    {
        for (int j = 0; j < si->width; j++)
        {
            int r = 0, g = 0, b = 0;
            for (int y = -bb; y <= bb; y++)
            {
                for (int x = -bb; x <= bb; x++)
                {
                    if (i + y < 0)
                        continue;
                    if (i + y >= si->height)
                        continue;
                    if (j + x < 0)
                        continue;
                    if (j + x >= si->width)
                        continue;
                    r += input_image->image_pixels[i + y][j + x][0];
                    g += input_image->image_pixels[i + y][j + x][1];
                    b += input_image->image_pixels[i + y][j + x][2];
                }
            }
            si->image_pixels[i][j][0] = r / ((2 * bb + 1) * (2 * bb + 1));
            si->image_pixels[i][j][1] = g / ((2 * bb + 1) * (2 * bb + 1));
            si->image_pixels[i][j][2] = b / ((2 * bb + 1) * (2 * bb + 1));
            // write(write_fd, si->image_pixels[i][j], 3 * sizeof(uint8_t));

            row[j].r = si->image_pixels[i][j][0];
            row[j].g = si->image_pixels[i][j][1];
            row[j].b = si->image_pixels[i][j][2];
            row[j].x = j;
            row[j].y = i;
            // write(write_fd, &p, sizeof(pixel));
            if (debug)
                cout << "S1 on Processed Pixel (" << p.x << "," << p.y << ")\n";
        }
        write(write_fd, row, si->width * sizeof(pixel));
    }
    // printf("%d", si->width);
}

void S2_find_details(struct image_t *input_image, int read_fd, int write_fd)
{
    int rows_read = 0;
    pixel *row = new pixel[input_image->width];
    pixel *out_row = new pixel[input_image->width];
    // TODO
    while (1)
    {

        ssize_t bytes_read = read(read_fd, &row, sizeof(pixel) * input_image->width);
        if (bytes_read == 0)
            continue;
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

        write(write_fd, &out_row, sizeof(pixel) * input_image->width);
        if (debug)
            cout << "S2 on Processed Pixel (" << out_p.x << "," << out_p.y << ")\n";
    }
}

void S3_sharpen(struct image_t *input_image, int read_fd)
{
    // TODO
    int pixels_read = 0;
    while (1)
    {
        pixel p;
        ssize_t bytes_read = read(read_fd, &p, sizeof(pixel));
        if (bytes_read == 0)
            continue;
        pixels_read++;
        if (pixels_read > input_image->width * input_image->height)
            break;
        if (debug)
            cout << "S3 on Received Pixel (" << p.x << "," << p.y << ")" << "Pixels Read: " << pixels_read << endl;

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
        if (pixels_read >= input_image->width * input_image->height)
            break;
    }
}
int main(int argc, char **argv)
{

    if (argc != 3)
    {
        if (debug)
            cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

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
    pid_t S1_pid = fork();

    if (S1_pid == 0)
    {
        close(S1_S2_pipe[0]);
        for (int i = 0; i < 1000; i++)
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
            for (int i = 0; i < 1000; i++)
            {
                S2_find_details(input_image, S1_S2_pipe[0], S2_S3_pipe[1]);
                cout << "Process S2 completed iteration " << i + 1 << "\n";
            }
        }
        else
        {
            close(S1_S2_pipe[0]);
            close(S2_S3_pipe[1]);
            for (int i = 0; i < 1000; i++)
            {
                S3_sharpen(input_image, S2_S3_pipe[0]);
                cout << "Process S3 completed iteration " << i + 1 << "\n";
            }
            write_ppm_file(argv[2], sharpened_image);
        }
    }
}