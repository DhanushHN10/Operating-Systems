#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <queue>
#include <sstream>
#include <algorithm>
using namespace std;

#define MAX_CPUS 2
bool cpu_busy[MAX_CPUS];
int running_process[MAX_CPUS];
void finished(int pid, int cpu_id);
void got_ready(int pid);
void terminate(int pid);

int terminated_processes = 0;
class Process
{
private:
    bool cpuBurst = true;

public:
    int arrival_time;
    vector<int> cpu_burst_times;
    vector<int> io_times;
    // io_times.push_back(0);
    string state = "ready";
    int proc_no;

    int turn_around_time;
    int cpu_time_rem;

    int cpu_index = 0;
    int io_index = 0;
    int io_time_rem;
    bool preempt = false;
    int cpu_id;
    long long got_ready_at;
    long long started_burst_at;
    vector<vector<long long>> burst_times;
    int pid;

    int bust_count = 0;

    Process(int arr_time, int id)
    {
        //io_times.push_back(0);
        arrival_time = arr_time;
        pid = id;
        burst_times = vector<vector<long long>>(0, vector<long long>(3, 0));
        got_ready_at = arr_time;
    }

    int add_cpu_time(int cpu_time)
    {
        cpu_burst_times.push_back(cpu_time);
        cout << "Added cput" << cpu_time << endl;
        cpu_time_rem = cpu_burst_times[0];
        return 0;
    }
    int add_IO_time(int IO_time)
    {
        io_times.push_back(IO_time);
        cout << "Added iot" << IO_time << endl;
        io_time_rem = io_times[0];
        return 0;
    }

    void update(long long time)
    {
        if (state == "running")
        {
            if (cpu_time_rem > 0)
            {
                cpu_time_rem--;
            }
            if (cpu_time_rem == 0)
            {
                finished(pid, cpu_id);
                burst_times.push_back({started_burst_at, time, cpu_id});
                cpu_index++;
                if (cpu_index < cpu_burst_times.size())
                {
                    state = "waiting";
                    io_time_rem = io_times[io_index];
                    io_index++;
                }
                else
                {
                    state = "terminated";
                    turn_around_time = time - arrival_time;
                    terminate(pid);
                }
            }
        }

        else if (state == "waiting")
        {
            if (io_time_rem > 0)
            {
                io_time_rem--;
            }
            if (io_time_rem == 0)
            {
                state = "ready";
                if (cpu_index < cpu_burst_times.size())
                {
                    got_ready(pid);
                    cpu_time_rem = cpu_burst_times[cpu_index];
                }
                else
                {
                    state = "terminated";
                }
            }
        }
    }

    int get_next_cpu_burst()
    {
        if (preempt || state == "running")
            return cpu_time_rem;
        if (cpu_index < cpu_burst_times.size())
            return cpu_burst_times[cpu_index];
        return -1;
    }

    void run(long long time, int cpu_no)
    {
        state = "running";
        started_burst_at = time;
        cpu_id = cpu_no;
    }

    void stop(long long time)
    {
        state = "ready";
        burst_times.push_back({started_burst_at, time, cpu_id});
        got_ready_at = time;
        cpu_id = -1;
    }
};

class comp
{
public:
    bool operator()(Process *p1, Process *p2)
    {
        return p1->get_next_cpu_burst() > p2->get_next_cpu_burst();
    }
};

priority_queue<Process *, vector<Process *>, comp> prq;
vector<Process> processes;

void finished(int pid, int cpu_id)
{

    cpu_busy[cpu_id] = false;
    running_process[cpu_id] = -1;
}

void got_ready(int pid)
{
    prq.push(&processes[pid - 1]);
}

void terminate(int pid)
{
    terminated_processes++;
}
void getInput(string filename)
{
    ifstream file(filename);
    string line;
    getline(file, line);
    while (line[0] == '<')
    {
        getline(file, line);
    }
    int pid = 1;

    while (1)
    {
        stringstream ss;
        ss << line;
        if (line[0] == '<')
            break;
        int x;
        ss >> x;
        Process np(x, pid);
        pid++;
        bool cpu_time = true;
        ss >> x;
        while (x != -1)
        {
            if (cpu_time)
            {
                np.add_cpu_time(x);
                cpu_time = !cpu_time;
            }
            else
            {
                np.add_IO_time(x);
                cpu_time = !cpu_time;
            }
            // break;
            ss >> x;
        }
        getline(file, line);

        processes.push_back(np);
    }
}

void nonpreemptivesjf(long long time, int cpu_id)
{
    if (cpu_busy[cpu_id])
    {
        cout << "[" << time << "]" << "CPU" << cpu_id << " is running " << running_process[cpu_id] << endl;
        return;
    }
    if (prq.empty())
        return;
    Process *p = prq.top();
    prq.pop();
    p->run(time, cpu_id);
    cpu_busy[cpu_id] = true;
    running_process[cpu_id] = p->pid;
    cout << "[" << time << "]" << "scheduled " << p->pid << "on " << cpu_id << endl;
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "you dumbeo enter 2\n";
    }

    long long time = 0;
    int idx = 0;
    int inpf = 0;
    getInput(argv[1]);
    for (int i = 0; i < MAX_CPUS; i++)
    {
        cpu_busy[i] = false;
        running_process[i] = -1;
    }
    while (1)
    {
        while (idx < processes.size() && processes[idx].arrival_time == time)
        {
            prq.push(&processes[idx]);
            cout << "[" << time << "]" << "process " << processes[idx].pid << " arrived" << endl;
            idx++;
            if (idx == processes.size())
                inpf = 1;
        }
        for (int i = 0; i < MAX_CPUS; i++)
            nonpreemptivesjf(time, i);
        for (int i = 0; i < processes.size(); i++)
        {
            processes[i].update(time);
        }

        time++;
        if (terminated_processes == processes.size() && inpf)
        {
            cout << "All processes terminated at time " << time << endl;
            break;
        }
        if (time == 30000)
            break;
        // break;
    }
    long long total_turnaround_time = 0;
    for (int i = 0; i < processes.size(); i++)
    {
        total_turnaround_time += processes[i].turn_around_time;
        cout << "Process " << processes[i].pid << " turnaround time " << processes[i].turn_around_time << endl;
    }
    cout << "Average turnaround time " << (double)total_turnaround_time / processes.size() << endl;

    vector<vector<vector<long long>>> gantt_times(MAX_CPUS, vector<vector<long long>>(0, vector<long long>(4, 0)));
    for (int i = 0; i < processes.size(); i++)
    {
        //cout << "Process " << processes[i].pid << "burst times" << endl;
        for (int j = 0; j < processes[i].burst_times.size(); j++)
        {
            int cpu_id = processes[i].burst_times[j][2];
            gantt_times[cpu_id].push_back({processes[i].burst_times[j][0], processes[i].burst_times[j][1], (long long)processes[i].pid, (long long)j + 1});
            //cout << processes[i].burst_times[j][0] << " " << processes[i].burst_times[j][1] << " on cpu " << processes[i].burst_times[j][2] << endl;
        }
    }
    ofstream gantt_file("gantt_chart.txt");
    for (int i = 0; i < MAX_CPUS; i++)
    {
        sort(gantt_times[i].begin(), gantt_times[i].end());
        gantt_file << "CPU" << i << endl;
        for (int j = 0; j < gantt_times[i].size(); j++)
        {
            gantt_file << "P" << gantt_times[i][j][2] << "," << gantt_times[i][j][3] << "\t\t" << gantt_times[i][j][0] << "," << gantt_times[i][j][1] << endl;
        }
    }

    gantt_file.close();
}
