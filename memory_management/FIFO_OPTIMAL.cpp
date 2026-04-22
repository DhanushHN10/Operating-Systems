#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <queue>
#include <functional>
#include <stdint.h>
using namespace std;

int free_frame = 0;
int NO_OF_FRAMES = 0;
int PAGE_SIZE = 0;

bool debug = false;
struct pair_hash
{
    size_t operator()(const pair<int, uint64_t> &p) const
    {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};
unordered_map<pair<int, uint64_t>, queue<long long>, pair_hash> future;
class Page
{
public:
    int pid;
    uint64_t page_number;
    uint64_t frame_number;

    Page(int p, uint64_t pn)
    {
        pid = p;
        page_number = pn;
        frame_number = -1;
    }
};

class Page_Table
{
public:
    int pid;
    int no_of_pages;
    int max_pages = 4;
    long page_faults;
    unordered_map<uint64_t, uint64_t> pages;
    queue<Page *> fifo_queue; // For FIFO
    string replacement_policy = "FIFO";

    Page_Table(int PID, int max_p, string policy)
    {
        pid = PID;
        no_of_pages = 0;
        max_pages = max_p;
        replacement_policy = policy;
        page_faults = 0;
    }
    bool page_exists(uint64_t page_number)
    {
        if (pages.find(page_number) != pages.end())
        {
            return true;
        }
        page_faults++;
        return false;
    }
    void add_page(Page *page)
    {
        pages[page->page_number] = page->frame_number;
        // free_frame++;
        fifo_queue.push(page);
        no_of_pages++;
    }

    void replace_page(uint64_t page_number)
    {

        if (replacement_policy == "FIFO")
        {
            // Replace the first in page
            Page *fifo_page = fifo_queue.front();
            fifo_queue.pop();
            pages.erase(fifo_page->page_number);
            pages[page_number] = fifo_page->frame_number;
            Page *page = new Page(pid, page_number);
            page->frame_number = fifo_page->frame_number;
            fifo_queue.push(page);
            if (debug)
                cout << "PID:" << pid << "Replaced page " << fifo_page->page_number << " with page " << page_number << " in frame " << page->frame_number << endl;
        }
    }
};
Page_Table *page_tables[4];
int frames_used = 0;

queue<Page *> global_fifo_queue;

void add_page(int pid, uint64_t page_number, string replacement_policy, string allocation_policy)
{
    // age_tables[pid]->add_page(page_number);
    if (replacement_policy == "FIFO")
    {
        if (allocation_policy == "LOCAL")
        {
            if (frames_used < NO_OF_FRAMES && page_tables[pid]->no_of_pages < page_tables[pid]->max_pages)
            {

                Page *page = new Page(pid, page_number);
                page->frame_number = pid * NO_OF_FRAMES / 4 + page_tables[pid]->no_of_pages;
                page_tables[pid]->add_page(page);
                free_frame++;
                frames_used++;
                if (debug)
                    cout << "PID:" << pid << " Added page " << page_number << " in frame " << page->frame_number << endl;
            }
            else
            {
                page_tables[pid]->replace_page(page_number);
            }
        }

        else if (allocation_policy == "GLOBAL")
        {
            if (frames_used < NO_OF_FRAMES)
            {
                Page *page = new Page(pid, page_number);
                page->frame_number = free_frame;
                page_tables[pid]->add_page(page);
                global_fifo_queue.push(page);
                free_frame++;
                frames_used++;
                if (debug)
                    cout << "PID:" << pid << " Added page " << page_number << " in frame " << page->frame_number << endl;
            }
            else
            {
                // Replace a page using global FIFO
                Page *fifo_page = global_fifo_queue.front();
                global_fifo_queue.pop();

                // Remove from old process
                Page_Table *old_table = page_tables[fifo_page->pid];
                old_table->pages.erase(fifo_page->page_number);
                // remove from fifo_queue too
                queue<Page *> newq;
                while (!old_table->fifo_queue.empty())
                {
                    Page *p = old_table->fifo_queue.front();
                    old_table->fifo_queue.pop();
                    if (p->page_number != fifo_page->page_number)
                        newq.push(p);
                }
                old_table->fifo_queue = newq;
                old_table->no_of_pages--;

                // Add new page
                Page *page = new Page(pid, page_number);
                page->frame_number = fifo_page->frame_number;
                page_tables[pid]->add_page(page);
                global_fifo_queue.push(page);

                if (debug)
                    cout << "Replaced PID:" << fifo_page->pid << " page " << fifo_page->page_number
                         << " with PID:" << pid << " page " << page_number
                         << " in frame " << fifo_page->frame_number << endl;
            }
        }
    }
    else if (replacement_policy == "OPTIMAL")
    {
        if (allocation_policy == "LOCAL")
        {
            if (frames_used < NO_OF_FRAMES && page_tables[pid]->no_of_pages < page_tables[pid]->max_pages)
            {

                Page *page = new Page(pid, page_number);
                page->frame_number = pid * NO_OF_FRAMES / 4 + page_tables[pid]->no_of_pages;
                page_tables[pid]->add_page(page);
                free_frame++;
                frames_used++;
                if (debug)
                    cout << "PID:" << pid << " Added page " << page_number << " in frame " << page->frame_number << endl;
            }
            else
            {
                long long farthest = -1;
                uint64_t page_to_replace = -1;
                for (auto P : page_tables[pid]->pages)
                {
                    if (future[{pid, P.first}].empty())
                    {
                        page_tables[pid]->pages.erase(P.first);
                        Page *page = new Page(pid, page_number);
                        page->frame_number = P.second;
                        page_tables[pid]->add_page(page);
                        if (debug)
                            cout << "PID:" << pid << "Replaced page " << P.first << " with page " << page_number << " in frame" << page->frame_number << endl;
                        return;
                    }
                    else if (future[{pid, P.first}].front() > farthest)
                    {
                        farthest = future[{pid, P.first}].front();
                        page_to_replace = P.first;
                    }
                }

                Page *page = new Page(pid, page_number);
                page->frame_number = page_tables[pid]->pages[page_to_replace];
                page_tables[pid]->pages.erase(page_to_replace);
                page_tables[pid]->add_page(page);
                if (debug)
                    cout << "PID:" << pid << "Replaced page " << page_to_replace << " with page " << page_number << " in frame" << page->frame_number << endl;
            }
        }

        else if (allocation_policy == "GLOBAL")
        {
            if (frames_used < NO_OF_FRAMES)
            {
                Page *page = new Page(pid, page_number);
                page->frame_number = free_frame;
                page_tables[pid]->add_page(page);
                global_fifo_queue.push(page);
                free_frame++;
                frames_used++;
                if (debug)
                    cout << "PID:" << pid << " Added page " << page_number << " in frame " << page->frame_number << endl;
            }
            else
            {
                long long farthest = -1;
                int victim_pid = -1;
                uint64_t page_to_replace = -1;

                // Find the global page to replace
                for (int p = 0; p < 4; p++)
                {
                    for (auto P : page_tables[p]->pages)
                    {
                        if (future[{p, P.first}].empty())
                        {
                            victim_pid = p;
                            page_to_replace = P.first;
                            break;
                        }
                        else if (future[{p, P.first}].front() > farthest)
                        {
                            farthest = future[{p, P.first}].front();
                            victim_pid = p;
                            page_to_replace = P.first;
                        }
                    }
                    if (victim_pid != -1 && page_to_replace != -1 && future[{victim_pid, page_to_replace}].empty())
                        break;
                }

                // Remove from old process
                Page_Table *old_table = page_tables[victim_pid];
                uint64_t frame_number = old_table->pages[page_to_replace];
                old_table->pages.erase(page_to_replace);
                // remove from fifo_queue too
                queue<Page *> newq;
                while (!old_table->fifo_queue.empty())
                {
                    Page *p = old_table->fifo_queue.front();
                    old_table->fifo_queue.pop();
                    if (p->page_number != page_to_replace)
                        newq.push(p);
                }
                old_table->fifo_queue = newq;
                old_table->no_of_pages--;

                // Add new page
                Page *page = new Page(pid, page_number);
                page->frame_number = frame_number;
                page_tables[pid]->add_page(page);
                global_fifo_queue.push(page);

                if (debug)
                    cout << "Replaced PID:" << victim_pid << " page " << page_to_replace
                         << " with PID:" << pid << " page " << page_number
                         << " in frame " << frame_number << endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    PAGE_SIZE = stoi(argv[1]);
    NO_OF_FRAMES = stoi(argv[2]);
    string replacement_policy = argv[3];
    string allocation_policy = argv[4];

    ifstream file = ifstream(argv[5]);
    string l;

    long long line_number = 0;
    if (file.is_open())
    {
        while (getline(file, l))
        {
            int pid = stoi(l.substr(0, 1));
            uint64_t address = stoull(l.substr(2));
            uint64_t page_number = address / PAGE_SIZE;

            if (future.find({pid, page_number}) == future.end())
            {
                future[{pid, page_number}] = queue<long long>();
                future[{pid, page_number}].push(line_number);
            }
            else
            {
                future[{pid, page_number}].push(line_number);
            }
            line_number++;
        }
    }
    string line;

    for (int i = 0; i < 4; i++)
    {
        page_tables[i] = new Page_Table(i, NO_OF_FRAMES / 4, replacement_policy);
    }
    uint64_t address;
    file = ifstream(argv[5]);

    if (file.is_open())
    {
        while (getline(file, line))
        {
            int pid = stoi(line.substr(0, 1));
            uint64_t address = stol(line.substr(2));
            uint64_t page_number = address / PAGE_SIZE;
            if (debug)
                cout << "PID: " << pid << " Address:" << address << endl;
            if (debug)
                cout << "Page Number: " << page_number << endl;
            future[{pid, page_number}].pop();
            if (page_tables[pid]->page_exists(page_number))
            {
                if (debug)
                    cout << "PID:" << pid << " Page " << page_number << " already exists in memory\n"
                         << endl;

                continue;
            }
            else
            {
                add_page(pid, page_number, replacement_policy, allocation_policy);
            }
            if (debug)
                cout << endl;
        }
        file.close();
        long total_faults = 0;

        for (int i = 0; i < 4; i++)
        {
            cout << "Process " << i << " had " << page_tables[i]->page_faults << " page faults." << endl;
            total_faults += page_tables[i]->page_faults;
        }
        cout << "Total Page Faults: " << total_faults << endl;
    }

    else
    {
        cout << "Error in opening the file" << endl;
    }
    return 0;
}
