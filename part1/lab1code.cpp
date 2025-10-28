#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>
using namespace std::chrono;
using namespace std;
int bb = 2;

struct image_t *S1_smoothen(struct image_t *input_image)
{
    // TODO
    // remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.
    struct image_t *si = new struct image_t;
    si->width = input_image->width;
    si->height = input_image->height;
    si->image_pixels = new uint8_t **[si->height];
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
        }
    }
    // printf("%d", si->width);
    return si;
}

struct image_t *S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
    // TODO
    struct image_t *de = new struct image_t;
    de->width = input_image->width;
    de->height = input_image->height;
    de->image_pixels = new uint8_t **[de->height];
    for (int i = 0; i < de->height; i++)
    {
        de->image_pixels[i] = new uint8_t *[de->width];
        for (int j = 0; j < de->width; j++)
            de->image_pixels[i][j] = new uint8_t[3];
    }

    for (int i = 0; i < de->height; i++)
    {
        for (int j = 0; j < de->width; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                int lol2 = input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k];
                if (lol2 > 0)
                    de->image_pixels[i][j][k] = lol2;
                else
                    de->image_pixels[i][j][k] = 0;
            }
        }
    }
    return de;
}

struct image_t *S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
    // TODO
    struct image_t *sh = new struct image_t;
    sh->width = input_image->width;
    sh->height = input_image->height;
    sh->image_pixels = new uint8_t **[sh->height];
    for (int i = 0; i < sh->height; i++)
    {
        sh->image_pixels[i] = new uint8_t *[sh->width];
        for (int j = 0; j < sh->width; j++)
            sh->image_pixels[i][j] = new uint8_t[3];
    }

    for (int i = 0; i < sh->height; i++)
    {
        for (int j = 0; j < sh->width; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                int lol = input_image->image_pixels[i][j][k] + details_image->image_pixels[i][j][k];
                if (lol > 255)
                    sh->image_pixels[i][j][k] = 255;
                else
                    sh->image_pixels[i][j][k] = lol;
            }
        }
    }
    return sh;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

    auto start = high_resolution_clock::now();

    struct image_t *input_image = read_ppm_file(argv[1]);
    auto rt = high_resolution_clock::now();
    auto rtduration = duration_cast<microseconds>(rt - start);
    cout << "Time taken to read image: " << rtduration.count() << " microseconds" << endl;

    struct image_t *smoothened_image = S1_smoothen(input_image);
    auto sm = high_resolution_clock::now();
    auto smduration = duration_cast<microseconds>(sm - rt);
    cout << "Time taken to smoothen image: " << smduration.count() << " microseconds" << endl;

    struct image_t *details_image = S2_find_details(input_image, smoothened_image);
    auto dt = high_resolution_clock::now();
    auto dtduration = duration_cast<microseconds>(dt - sm);
    cout << "Time taken to find details: " << dtduration.count() << " microseconds" << endl;

    struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
    auto sh = high_resolution_clock::now();
    auto shduration = duration_cast<microseconds>(sh - dt);
    cout << "Time taken to sharpen image: " << shduration.count() << " microseconds" << endl;

    write_ppm_file(argv[2], sharpened_image);
    auto wr = high_resolution_clock::now();
    auto wrduration = duration_cast<microseconds>(wr - sh);
    cout << "Time taken to write image: " << wrduration.count() << " microseconds" << endl;

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Total time taken: " << duration.count() << " microseconds" << endl;

    return 0;
}
