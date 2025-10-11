#include <iostream>
#include <unistd.h>
#include <cstring>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cout << "Usage: ./executable <page size> <number of frames> <replacement policy> <allocation policy> <trace file>" << endl;
        return 0;
    }

    // int page_size = stoi(argv[1]);
    // int num_frames = stoi(argv[2]);
    string replacement_policy = argv[3];
    string allocation_policy = argv[4];
    if (allocation_policy == "local")
    {
        allocation_policy = "LOCAL";
    }
    else if (allocation_policy == "global")
    {
        allocation_policy = "GLOBAL";
    }
    if (allocation_policy != "LOCAL" && allocation_policy != "GLOBAL")
    {
        cout << "Allocation policy should be either local or global" << endl;
        return 0;
    }

    string file = argv[5];

    if (replacement_policy == "FIFO" || replacement_policy == "OPTIMAL")
    {
        char *args[] = {strdup("./FIFO_OPTIMAL.out"), argv[1], argv[2], argv[3], argv[4], argv[5], NULL};
        execvp("./FIFO_OPTIMAL.out", args);
    }

    else if (replacement_policy == "LRU" || replacement_policy == "Random")
    {
        char *args[] = {strdup("./LRU_and_Random.out"), argv[1], argv[2], argv[3], argv[4], argv[5], NULL};
        execvp("./LRU_and_Random.out", args);
    }

    else
    {
        cout << "Replacement policy should be either FIFO, OPTIMAL, LRU or Random" << endl;
        return 0;
    }
    return 0;
}