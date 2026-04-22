//  PageTable class is for the Process and also contains other related variables, and doesn't mean it strictly means the PageTable itself. The Object of the class contains it's own pagetable.
#include <bits/stdc++.h>
// #include <chrono>
#include <random>
#include <cstdlib>
#include <ctime>

using namespace std;
// using namespace std::chrono;

int globalPageFaults = 0;
bool debug = false;
long long timer = 0;

std::random_device rd;

std::mt19937 gen(rd() ^ time(nullptr));

class FrameStatus
{
public:
    int frame_number = -1; // Required only for local allocation policy where i store the frame index directly in a processes's allocated frames vector...
    int pid = -1;
    uint64_t page_number = -1;
    long long lastAccessed = -1;

    FrameStatus(int frame_number)
    {
        this->frame_number = frame_number;
    }

    void assign(int pid, uint64_t page_number, long long &timer)
    {
        this->pid = pid;
        this->page_number = page_number;
        this->lastAccessed = timer;
    }

    void reset()
    {
        this->pid = -1;
        this->page_number = -1;
        this->lastAccessed = -1;
    }

    bool isFree()
    {
        return (pid == -1 && page_number == -1);
    }

    void updateAccessTimer(long long &timer)
    {
        this->lastAccessed = timer;
    }
};

class PageTable
{

public:
    int pid;
    int perProcess_pageFaults = 0;
    int maxFrames = -1;
    int lastAccesed = -1;
    unordered_map<uint64_t, int> pageTable;
    vector<int> allocatedFrames; // Storing the frame number allocated to the process. Needed for fast access and eviction in case of local allocation policy.

    PageTable(int pid)
    {
        this->pid = pid;
    }
    PageTable(int pid, int noFrames)
    {
        this->pid = pid;
        this->maxFrames = noFrames;
    }
    int getPID()
    {
        return this->pid;
    }

    void pagefault()
    {
        globalPageFaults++;
        perProcess_pageFaults++;
    }

    int getProcessPageFaults()
    {
        return perProcess_pageFaults;
    }

    bool isPresent(uint64_t page_number)
    {
        return (pageTable.find(page_number) != pageTable.end());
    }

    void evictPage(uint64_t page_number)
    {
        pageTable.erase(page_number);
    }
};

int findLRUFrame(vector<FrameStatus *> &frame_table)
{
    int minIndex = -1;
    long long minTime = LLONG_MAX;
    for (int i = 0; i < frame_table.size(); ++i)
    {
        if (frame_table[i]->lastAccessed < minTime)
        {
            minTime = frame_table[i]->lastAccessed;
            minIndex = i;
        }
    }

    return minIndex;
}

int findLRUFrame_local_policy(vector<FrameStatus *> &frame_table, PageTable *&process)
{
    int idx = -1;
    long long minTime = LLONG_MAX;

    for (int i = 0; i < process->allocatedFrames.size(); ++i)
    {
        int frameIdx = process->allocatedFrames[i];

        if (frame_table[frameIdx]->lastAccessed < minTime)
        {
            minTime = frame_table[frameIdx]->lastAccessed;
            idx = frameIdx;
        }
    }

    return idx;
}

void LRU(int &pid, uint64_t &page_number, vector<FrameStatus *> &frame_table, vector<PageTable *> &page_tables, long long &timer, string &allocationPolicy)
{

    if (debug)
    {
        cout << "Eviction needed for process " << pid << " for page number " << page_number << endl;
    }
    if (allocationPolicy == "global")
    {
        int lruIndex = findLRUFrame(frame_table);
        int old_pid = frame_table[lruIndex]->pid;
        uint64_t old_page = frame_table[lruIndex]->page_number;

        page_tables[old_pid]->evictPage(old_page);

        frame_table[lruIndex]->reset();
        frame_table[lruIndex]->assign(pid, page_number, timer);
        page_tables[pid]->pageTable[page_number] = lruIndex;
    }

    else if (allocationPolicy == "local")
    {

        PageTable *process = page_tables[pid];
        int lruIndex = findLRUFrame_local_policy(frame_table, process);

        int old_pid = frame_table[lruIndex]->pid;
        uint64_t old_page = frame_table[lruIndex]->page_number;

        page_tables[old_pid]->evictPage(old_page);

        frame_table[lruIndex]->reset();
        frame_table[lruIndex]->assign(pid, page_number, timer);
        page_tables[pid]->pageTable[page_number] = lruIndex;
    }
}

void Random(int &pid, uint64_t &page_number, vector<FrameStatus *> &frame_table, vector<PageTable *> &page_tables, long long &timer, string &allocationPolicy, int &numFrames)
{
    if (debug)
    {
        cout << "Eviction needed for process " << pid << " for page number " << page_number << endl;
    }

    if (allocationPolicy == "global")
    {

        std::uniform_int_distribution<> dist(0, numFrames - 1);
        // int victim = rand() % (numFrames);
        int victim = dist(gen);
        // cout<<"Victim : "<<victim<<endl;

        int old_pid = frame_table[victim]->pid;
        uint16_t old_page = frame_table[victim]->page_number;

        page_tables[old_pid]->evictPage(old_page);

        frame_table[victim]->reset();
        frame_table[victim]->assign(pid, page_number, timer);
        page_tables[pid]->pageTable[page_number] = victim;
    }

    else if (allocationPolicy == "local")
    {

        PageTable *process = page_tables[pid];
        std::uniform_int_distribution<> dist(0, process->allocatedFrames.size() - 1);
        int randIdx = dist(gen);
        // int randIdx = rand() % (process->allocatedFrames.size());
        int victim = process->allocatedFrames[randIdx];

        int old_pid = frame_table[victim]->pid;
        uint64_t old_page = frame_table[victim]->page_number;

        page_tables[old_pid]->evictPage(old_page);

        frame_table[victim]->reset();
        frame_table[victim]->assign(pid, page_number, timer);
        page_tables[pid]->pageTable[page_number] = victim;
    }
}
int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cout << "Enter the correct number of arguments\n"
             << "<page-size>\n"
             << "<number-of-memory-frames> <replacement-policy> <allocation-policy>\n"
             << "<path-to-memory-trace-file>" << endl;

        exit(0);
    }

    // srand(time(nullptr) ^ clock());

    int pageSize = atoi(argv[1]);
    int memFrames = atoi(argv[2]);
    string replacementPolicy = argv[3];
    string allocationPolicy = argv[4];
    int freeFrameIdx = 0;

    ifstream file(argv[5]);
    string line;
    if (!file)
    {
        cout << "File not found" << endl;
        exit(0);
    }

    int localFrames = memFrames / 4;

    vector<FrameStatus *> frame_table;
    for (int i = 0; i < memFrames; ++i)
    {
        frame_table.push_back(new FrameStatus(i));
    }

    vector<PageTable *> page_tables;
    for (int i = 0; i < 4; ++i)
    {
        if (allocationPolicy == "local")
            page_tables.push_back(new PageTable(i, localFrames));
        else if (allocationPolicy == "global")
            page_tables.push_back(new PageTable(i));
        else
        {
            cout << "Allocation Policy should either be local or global" << endl;
            exit(0);
        }
    }

    if (file.is_open())
    {
        while (getline(file, line))
        {
            int pid = stoi(line.substr(0, 1));
            // cout<< line.substr(2)<<endl;
            uint64_t address = stoull(line.substr(2));
            if (debug)
                cout << "pid: " << pid << " Adderss: " << address << endl;

            uint64_t page_number = address / pageSize;

            PageTable *pt = page_tables[pid];
            if (!(pt->isPresent(page_number)))
            {

                pt->pagefault();
                if (debug)
                {
                    // cout<<"Compulsory Miss"<<endl;
                    cout << "Page Fault for process " << pid << " for page number: " << page_number << endl;
                }

                if (freeFrameIdx < frame_table.size())
                {

                    if (allocationPolicy == "local")
                    {
                        if (pt->allocatedFrames.size() < pt->maxFrames)
                        {
                            frame_table[freeFrameIdx]->assign(pid, page_number, timer);
                            pt->pageTable[page_number] = freeFrameIdx;
                            pt->allocatedFrames.push_back(freeFrameIdx);
                            freeFrameIdx++;
                        }
                        else
                        {

                            if (replacementPolicy == "LRU")
                            {
                                LRU(pid, page_number, frame_table, page_tables, timer, allocationPolicy);
                            }

                            else if (replacementPolicy == "Random")
                            {
                                Random(pid, page_number, frame_table, page_tables, timer, allocationPolicy, memFrames);
                            }
                        }
                    }

                    else if (allocationPolicy == "global")
                    {
                        frame_table[freeFrameIdx]->assign(pid, page_number, timer);
                        pt->pageTable[page_number] = freeFrameIdx;
                        freeFrameIdx++;
                    }
                }

                else
                {

                    if (replacementPolicy == "LRU")
                    {
                        LRU(pid, page_number, frame_table, page_tables, timer, allocationPolicy);
                    }

                    else if (replacementPolicy == "Random")
                    {
                        Random(pid, page_number, frame_table, page_tables, timer, allocationPolicy, memFrames);
                    }
                    else
                    {
                        cout << "Replacement Policy should either be LRU or Random" << endl;
                        exit(0);
                    }
                }
            }

            else
            {

                int frameIdx = pt->pageTable[page_number];
                frame_table[frameIdx]->updateAccessTimer(timer);
                if (debug)
                {
                    cout << "Page Hit for process " << pid << " for page number: " << page_number << endl;
                }
            }

            if (debug)
                cout << endl;

            timer++;
        }
        file.close();

        cout << "Total Page Faults: " << globalPageFaults << endl;
        for (int i = 0; i < 4; ++i)
        {
            cout << "Process: " << i << endl;
            cout << "Page Faults: " << page_tables[i]->getProcessPageFaults() << endl;
        }
    }

    return 0;
}