#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <queue>
#include <sstream>
#include <set>
#include <bits/stdc++.h> 
#include <chrono>
using namespace std::chrono;

using namespace std;
using ll = long long;

ll timer = 0;
bool debugMode=false;

class Process
{
private:
    bool cpuBurst;

public:
    int arrival_time;
    vector<int> cpu_burst_times;
    vector<int> io_times;
    string state;
    int proc_no;
    int cpu_time_rem;
    int cpu_index;
    int io_index;
    int io_time_rem;
    int no_time_on_cpu;
    int temp_start_time;
    int temp_end_time;

    ll turn_around_time;
    int active_time;

    Process(int arr_time)
    {
        // io_times.push_back(0);
        //     arrival_time = arr_time;
        //     cpu_index=0;
        // io_index=0;
        cpuBurst = true;
        arrival_time = arr_time;
        cpu_index = 0;
        io_index = 0;
        cpu_time_rem = 0;
        io_time_rem = 0;
        turn_around_time = 0;
        active_time = 0;
        state = "ready";
        no_time_on_cpu = 0;
        temp_start_time = 0;
        temp_end_time = 0;
    }

    int add_cpu_time(int cpu_time)
    {

        cpu_burst_times.push_back(cpu_time);
        if (cpu_burst_times.size() == 1)
            cpu_time_rem = cpu_burst_times[0];
        if(debugMode)cout << "Added CPU burst time: " << cpu_time << endl;
        return 0;
    }
    int add_IO_time(int IO_time)
    {
        io_times.push_back(IO_time);
        if (io_times.size() == 1)
            io_time_rem = io_times[0];
        if(debugMode) cout<< "Added I/O burst time: " << IO_time << endl;
        return 0;
    }

    void update()
    {
        if (state == "running")
        {
            if (cpu_time_rem > 0)
            {
                cpu_time_rem--;
                active_time++;
            }
            if (cpu_time_rem == 0)
            {
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
                }
            }
        }

        else if (state == "waiting")
        {
            if (io_time_rem > -1)
            {
                io_time_rem--;
                active_time++;
            }
            if (io_time_rem == -1)
            {
                state = "ready";
                if (cpu_index < cpu_burst_times.size())
                {
                    cpu_time_rem = cpu_burst_times[cpu_index];
                }
                else
                {
                    state = "terminated";
                }
            }
        }
    }
};

void update_waiting_processes(set<Process *> &waiting_processes, queue<Process *> &process_queue, ll &timer)
{
    if (waiting_processes.empty())
        return;
    for (auto it = waiting_processes.begin(); it != waiting_processes.end();)
    {
        Process *proc = *it;
        if (proc->state == "waiting")
        {
            proc->update();
        }

        if (proc->state == "ready")
        {
            process_queue.push(proc);
            it = waiting_processes.erase(it);
        }
        else if (proc->state == "terminated")
        {
           if(debugMode) cout << "Process " << proc->proc_no << " had terminated" << " at time : " << timer << endl;
            it = waiting_processes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
void fifo(queue<Process *> &process_queue, Process *&curr_process, set<Process *> &waiting_processes, ll &timer, vector<vector<int>> &gantt_chart, int cpu_no)
{
    // queue<Process> ready_queue;
    // Process* curr_process = nullptr;
    // while(1){

    // return;

    if (curr_process)
    {
        curr_process->update();
    }
    // cout<<curr_process->proc_no<<endl;}
    // update_waiting_processes(waiting_processes, process_queue, timer);

    if (!curr_process && process_queue.empty())
    {
        return;
    }
    if (curr_process == nullptr && !process_queue.empty())
    {
        // cout<<timer<<endl;
        curr_process = process_queue.front();
        // ready_queue.pop();
        // curr_process->state="running";
        curr_process->state = "running";
        // curr_process->update();
        process_queue.pop();
        // int proc_temp_start_time= timer;
        curr_process->temp_start_time = timer;
        curr_process->no_time_on_cpu++;

        curr_process->update();

       if(debugMode) cout << "Process " << curr_process->proc_no << " is now running at time : " << timer << endl;
    }

    if (curr_process && curr_process->state == "waiting")
    {
        if(debugMode) cout << "Process " << curr_process->proc_no << " is now blocked and waiting due to I/O; at time : " << timer << endl;
        // process_queue.pop();

        // curr_process->update();
        waiting_processes.insert(curr_process);
        curr_process->temp_end_time = timer;
        auto stat = {cpu_no, curr_process->proc_no, curr_process->no_time_on_cpu, curr_process->temp_start_time, curr_process->temp_end_time};
        gantt_chart.push_back(stat);
        curr_process = nullptr;
    }

    if (curr_process && curr_process->state == "terminated")
    {
       if(debugMode) cout << "Process " << curr_process->proc_no << " has terminated; at time : " << timer << endl;
        curr_process->turn_around_time = timer - curr_process->arrival_time + 1;
        // process_queue.pop();
        curr_process->temp_end_time = timer;
        auto stat = {cpu_no, curr_process->proc_no, curr_process->no_time_on_cpu, curr_process->temp_start_time, curr_process->temp_end_time};
        gantt_chart.push_back(stat);
        curr_process = nullptr;
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        cout << "Enter the <input_file>\n";
        exit(0);
    }

    int no_cpus = 2;
    vector<Process *> curr_processes(no_cpus, nullptr);
    ;
    set<Process *> waiting_processes;
    vector<Process *> all_processes;
    queue<Process *> process_queue;
    double average_turnaround_time = 0;
    vector<vector<int>> gantt_chart;

    ifstream file(argv[1]);
    string line;

    getline(file, line);
    while (line[0] == '<')
    {
        // cout<<line<<endl;
        getline(file, line);
    }


     auto start = high_resolution_clock::now();
    while (1)
    {
        int arrival_time;
        stringstream ss;
        ss << line;
        // if(line[0] == '<')
        // break;

        ss >> arrival_time;
        // cout<<ss.str()<<endl;
        // cout<<arrival_time<<endl;
        while (1LL * arrival_time == timer)
        {
            Process *new_process = new Process(timer);

            new_process->proc_no = all_processes.size() + 1;
            // cout<<new_process->proc_no<<endl;
            // Process np(timer);
            int x;
            bool cpu_time = true;
            ss >> x;

            // cout<<ss<<endl;

            while (x != -1)
            {
                if (cpu_time)
                {
                    new_process->add_cpu_time(x);
                    cpu_time = !cpu_time;
                }
                else
                {
                    new_process->add_IO_time(x);
                    cpu_time = !cpu_time;
                }

                ss >> x;
                // cout<<ss.str()<<endl;
                // cout<<line<<endl;
            }

            process_queue.push(new_process);
            all_processes.push_back(new_process);
           

            // getline(file, line);
            if (!getline(file, line))
            {
                line = "";
                break;
            }
            stringstream ss2(line);
            ss2 >> arrival_time;

            // cout<<line[0]<<endl;
            if (line[0] == '<')
                break;
        }

        for (int i = 0; i < no_cpus; ++i)
        {
            fifo(process_queue, curr_processes[i], waiting_processes, timer, gantt_chart, i);
        }
        update_waiting_processes(waiting_processes, process_queue, timer);

        bool all_idle = true;
        for (auto *c : curr_processes)
        {
            if (c != nullptr)
            {
                all_idle = false;
                break;
            }
        }
        if (process_queue.empty() && waiting_processes.empty() && all_idle && (line[0] == '<' || file.eof() || line.empty()))
        {

            cout << "All processes completed at time : " << timer << endl;
            for (auto &proc : all_processes)
            {
                cout << "Process " << proc->proc_no << " Turnaround time: " << proc->turn_around_time << endl;
                average_turnaround_time += proc->turn_around_time;
            }

            double size = all_processes.size();
            average_turnaround_time /= size;
            cout << "\nAverage Turnaround time: " << average_turnaround_time << endl;
            break;
        }

        timer++;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Total time taken: " << duration.count() << " microseconds" << endl;



    // cout<<"Is queue empty: "<<process_queue.empty()<<endl;
    // cout<<"Is waiting empty: "<<waiting_processes.empty()<<endl;
    // if(!process_queue.empty())cout<<process_queue.front()->proc_no<<endl;
    // if(curr_process)cout<<curr_process->proc_no<<endl;
    // cout<<"Is current process null: "<<(curr_process==nullptr)<<endl;

    sort(gantt_chart.begin(), gantt_chart.end(), [](const vector<int> &a, const vector<int> &b)
         {
    if(a[0] != b[0]) return a[0] < b[0];  
    return a[3] < b[3]; });

    ofstream gantt_file("gantt_chart_fifo_multi_core.txt");

    for (int cpu_id = 0; cpu_id < no_cpus; cpu_id++)
    {
        gantt_file << "CPU" << cpu_id << "\n";
        for (auto &stat : gantt_chart)
        {
            if (stat[0] == cpu_id)
            {
                gantt_file << "P" << stat[1] << "," << stat[2] << "\t"
                           << stat[3] << "\t" << stat[4] << "\n";
            }
        }
    }

    return 0;
}
