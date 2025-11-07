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

void S1_smoothen(struct image_t *input_image, void *S1_S2_ptr, sem_t *S1_S2_sem, sem_t *S2_S1_sem)
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

        sem_wait(S2_S1_sem);
        memcpy(S1_S2_ptr, &i, sizeof(int));
        memcpy(((char *)S1_S2_ptr) + sizeof(int), row, sizeof(pixel) * input_image->width);
        sem_post(S1_S2_sem);
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

void S2_find_details(struct image_t *input_image, void *S1_S2_ptr, void *S2_S3_ptr, sem_t *S1_S2_sem, sem_t *S2_S3_sem, sem_t *S2_S1_sem, sem_t *S3_S2_sem)
{
    int rows_read = 0;

    // TODO
    pixel *row = new pixel[input_image->width];
    pixel *out_row = new pixel[input_image->width];
    while (1)
    {

        char *readp = (char *)row;

        size_t bytes_read = 0;
        int row_index;

        // ssize_t bytes_read = read(read_fd, readp, sizeof(pixel) * input_image->width);
        sem_wait(S1_S2_sem);
        memcpy(&row_index, S1_S2_ptr, sizeof(int));
        if (debug)
            cout << "S2 got row index " << row_index << endl;
        // sem_post(S2_S1_sem);

        if (row_index != rows_read)
        {
            if (debug)
                cout << "S2 row index duplicate" << "Expected " << rows_read << endl;
            // sem_post(S1_S2_sem);
            continue;
        }
        else
        {
            *((int *)S1_S2_ptr) = -1;
        }
        rows_read++;
        // sem_wait(S1_S2_sem);
        memcpy(row, ((char *)S1_S2_ptr) + sizeof(int), sizeof(pixel) * input_image->width);
        sem_post(S2_S1_sem);

        if (debug)
            cout << "S2:" << row[0].x << " " << row[0].y << " " << (int)row[0].r << " " << (int)row[0].g << " " << (int)row[0].b << endl;

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

        sem_wait(S3_S2_sem);
        memcpy(S2_S3_ptr, &row_index, sizeof(int));
        memcpy(((char *)S2_S3_ptr) + sizeof(int), out_row, sizeof(pixel) * input_image->width);
        sem_post(S2_S3_sem);
        // delete[] row;
        // delete[] out_row;
        // while (1)
        // {
        //     sem_wait(S2_S3_sem);
        //     int x = *((int *)S2_S3_ptr);
        //     // cout << "S1 sees row index in shm: " << x << endl;
        //     sem_post(S2_S3_sem);
        //     // break;
        //     if (x == -1)
        //         break;
        // }

        if (debug)
            cout << "S2 on Processed row (" << rows_read << ")\n";
        if (rows_read >= input_image->height)
            break;
    }
    delete[] row;
    delete[] out_row;
}

void S3_sharpen(struct image_t *input_image, void *S2_S3_ptr, sem_t *S2_S3_sem, sem_t *S3_S2_sem)
{
    // TODO
    int rows_read = 0;
    pixel *row = new pixel[input_image->width];

    while (1)
    {
        char *readp = (char *)row;

        pixel p;
        size_t bytes_read = 0;

        int row_index;

        // ssize_t bytes_read = read(read_fd, readp, sizeof(pixel) * input_image->width);
        sem_wait(S2_S3_sem);
        memcpy(&row_index, S2_S3_ptr, sizeof(int));
        if (debug)
            cout << "S3 got row index " << row_index << endl;
        // sem_post(S3_S2_sem);

        if (row_index != rows_read)
        {
            if (debug)
                cout << "S3 row index duplicate" << rows_read << endl;
            // sem_post(S2_S3_sem);
            continue;
        }
        *((int *)S2_S3_ptr) = -1;
        rows_read++;
        // sem_wait(S2_S3_sem);
        memcpy(row, ((char *)S2_S3_ptr) + sizeof(int), sizeof(pixel) * input_image->width);
        sem_post(S3_S2_sem);
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

    if (argc != 3)
    {
        if (debug)
            cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

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

    const char *name = "/S1_S2";
    const char *name2 = "/S2_S3";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    int shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1 || shm_fd2 == -1)
    {
        perror("shm_open");
        return 1;
    }

    size_t shm_size = input_image->width * (sizeof(pixel) + sizeof(int)) * 1;
    if (ftruncate(shm_fd, shm_size) == -1 || ftruncate(shm_fd2, shm_size) == -1)
    {
        perror("ftruncate");
        return 1;
    }

    void *S1_S2 = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    void *S2_S3 = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    *((int *)S1_S2) = -1;
    *((int *)S2_S3) = -1;

    sem_unlink("/S1_S2_sem"); // remove the semaphore if it exists
    sem_unlink("/S2_S3_sem");
    sem_unlink("/S2_S1_sem");
    sem_unlink("/S3_S2_sem");
    sem_t *S1_S2_sem = sem_open("/S1_S2_sem", O_CREAT | O_EXCL, 0666, 0);
    sem_t *S2_S3_sem = sem_open("/S2_S3_sem", O_CREAT | O_EXCL, 0666, 0);
    sem_t *S2_S1_sem = sem_open("/S2_S1_sem", O_CREAT | O_EXCL, 0666, 1);
    sem_t *S3_S2_sem = sem_open("/S3_S2_sem", O_CREAT | O_EXCL, 0666, 1);

    // sem_post(S2_S1_sem);

    // if (S1_S2_sem <= 0 || S1_S2_sem == SEM_FAILED || S2_S3_sem <= 0 || S2_S3_sem == SEM_FAILED)
    // {
    //     perror("semaphore open failed");
    //     return 1;
    // }
    cout << "Height:" << height << endl;
    auto start = chrono::high_resolution_clock::now();
    pid_t S1_pid = fork();

    if (S1_pid == 0)
    {
        cout << "S1 process:" << getpid() << endl;

        for (int i = 0; i < noi; i++)
        {
            S1_smoothen(input_image, S1_S2, S1_S2_sem, S2_S1_sem);
            cout << "Process S1 completed iteration " << i + 1 << "\n";
        }

        exit(0);
    }
    else
    {
        pid_t S2_pid = fork();
        if (S2_pid == 0)
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cout << "S2 process:" << getpid() << endl;

            for (int i = 0; i < noi; i++)
            {
                S2_find_details(input_image, S1_S2, S2_S3, S1_S2_sem, S2_S3_sem, S2_S1_sem, S3_S2_sem);
                cout << "Process S2 completed iteration " << i + 1 << "\n";
            }

            exit(0);
        }
        else
        {

            cout << "S3 process:" << getpid() << endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds(2));

            for (int i = 0; i < noi; i++)
            {
                S3_sharpen(input_image, S2_S3, S2_S3_sem, S3_S2_sem);
                cout
                    << "Process S3 completed iteration " << i + 1 << "\n";
            }
            auto stop = chrono::high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            cout << "Time for " << noi << "  iterations:" << duration.count() << "milliseconds" << endl;
            waitpid(S1_pid, NULL, 0);
            waitpid(S2_pid, NULL, 0);
            write_ppm_file(argv[2], sharpened_image);

            exit(0);
        }
    }
}