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
#include<chrono> 

using namespace std::chrono;
using namespace std;
using ll = long long;
bool debugMode = false;




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
    int run_time_counter;

    ll turn_around_time;
    int active_time;
    int temp_start_time;
    int no_time_on_cpu;
    int temp_end_time;

    Process(int arr_time)
    {
        cpuBurst = true;
        arrival_time = arr_time;
        cpu_index = 0;
        io_index = 0;
        cpu_time_rem = 0;
        io_time_rem = 0;
        turn_around_time = 0;
        active_time = 0;
        run_time_counter=0;
        state = "ready";
        no_time_on_cpu=0;
    }

    int add_cpu_time(int cpu_time)
    {

        cpu_burst_times.push_back(cpu_time);
        if (cpu_burst_times.size() == 1)
            cpu_time_rem = cpu_burst_times[0];
       if(debugMode)  cout << "Added CPU burst time: " << cpu_time << endl;
        return 0;
    }
    int add_IO_time(int IO_time)
    {
        io_times.push_back(IO_time);
        if (io_times.size() == 1)
            io_time_rem = io_times[0];
      if(debugMode)  cout << "Added I/O burst time: " << IO_time << endl;
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
                run_time_counter++;
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

void update_waiting_processes(set<Process*>& waiting_processes, queue<Process*>& process_queue, ll& timer)
{
    if(waiting_processes.empty())return;
    for(auto it=waiting_processes.begin(); it!=waiting_processes.end();)
    {
        Process* proc = *it;
        if(proc->state=="waiting")
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
          if(debugMode)  cout << "Process " << proc->proc_no << " had terminated" <<" at time : "<<timer<< endl;
            it = waiting_processes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void roundRobin(queue<Process*>& process_queue, Process*& curr_process,set<Process*>& waiting_processes, ll& timer, int& time_quanta, vector<vector<int>>& gantt_chart)
{
    if(curr_process) curr_process->update();
    update_waiting_processes(waiting_processes,process_queue,timer);

    if(!curr_process && process_queue.empty()) 
    return;

    if(curr_process == nullptr && !process_queue.empty())
    {
        curr_process = process_queue.front();
        process_queue.pop();
        curr_process->state= "running";
        curr_process->run_time_counter=0;
      if(debugMode)  cout<<"Process "<<curr_process->proc_no<<" is now running at time : "<<timer<<endl;
        curr_process->temp_start_time=timer;
        curr_process->no_time_on_cpu++;
        curr_process->update();

    }

    if(curr_process && curr_process->run_time_counter == time_quanta && curr_process->state=="running")
    {
     if(debugMode)   cout<<"Time quanta expired for process: "<<curr_process->proc_no<<
        " at time: "<<timer<<endl;
        curr_process->state="ready";
        curr_process->temp_end_time=timer;
        auto stat = {curr_process->proc_no, curr_process->no_time_on_cpu,curr_process->temp_start_time, curr_process->temp_end_time};
        gantt_chart.push_back(stat);
        process_queue.push(curr_process);
        curr_process->run_time_counter=0;
        curr_process= nullptr;
        

    }

    if(curr_process && curr_process->state=="waiting")
    {
      if(debugMode)  cout<<"Process "<<curr_process->proc_no<<" is now blocked and waiting due to I/O; at time : "<<timer<<endl;
        waiting_processes.insert(curr_process);
        curr_process->temp_end_time=timer;
        auto stat= {curr_process->proc_no,curr_process->no_time_on_cpu, curr_process->temp_start_time,curr_process->temp_end_time };
        gantt_chart.push_back(stat);
        curr_process= nullptr;

    }

    if(curr_process && curr_process->state=="terminated")
    {
      if(debugMode)  cout<<"Process "<<curr_process->proc_no<<" has terminated at time : "<<timer<< endl;
        curr_process->turn_around_time= timer - curr_process->arrival_time + 1 ;
        curr_process->temp_end_time=timer;
        auto stat= {curr_process->proc_no,curr_process->no_time_on_cpu, curr_process->temp_start_time,curr_process->temp_end_time };
        gantt_chart.push_back(stat);
        curr_process=nullptr;
    }
}




int main(int argc, char* argv[])
{
     if(argc != 3)
    {
        cout<<"Enter the required argument- <input_file> <time_quanta>\n";
    }
    
    int time_quanta = stoi(argv[2]);
    ifstream file(argv[1]);
    string line;
    getline(file, line);
    while(line[0] == '<')
    {
        
        getline(file, line);   
    }

    vector<Process*> all_processes;
    queue<Process*> process_queue;
    set<Process*> waiting_processes;
    Process* curr_process = nullptr;  
    double average_turnaround_time =0;


    while(1)
    {
        int arrival_time;
        stringstream ss;
        ss << line;
        if(line[0]=='<')break;

        ss >> arrival_time;
        Process* new_process = new Process(arrival_time);
        new_process->proc_no = all_processes.size() + 1;
        int x;
        bool cpu_time = true;
        ss >> x;
        while(x != -1)
            {
                if(cpu_time)
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
               
                // cout<<line<<endl;
            }

            all_processes.push_back(new_process);
             if(!getline(file, line))
            {
                line="";
                break;
            }

    }

   
    //  Pre Loaded all the process's data

    // Impelementing the RR scheduling algo
    int process_idx=0;
   

    ll timer = 0;
    vector<vector<int>> gantt_chart;
    auto start = high_resolution_clock::now();
    while(!process_queue.empty() || !waiting_processes.empty() || curr_process != nullptr ||  process_idx < all_processes.size())
    {
        if(process_idx < all_processes.size()){
        while(all_processes[process_idx]->arrival_time == timer )
        {
            Process* p= all_processes[process_idx];
            process_queue.push(p);
            process_idx++;
             if(process_idx==all_processes.size())break;
        }}


        roundRobin(process_queue,curr_process,waiting_processes,timer,time_quanta,gantt_chart);
    //         cout<<"Is queue empty: "<<process_queue.empty()<<endl;
    // cout<<"Is waiting empty: "<<waiting_processes.empty()<<endl;
    // if(!process_queue.empty())cout<<process_queue.front()->proc_no<<endl;
    // if(curr_process)cout<<curr_process->proc_no<<endl;
    // cout<<"Is current process null: "<<(curr_process==nullptr)<<endl;






        timer++;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);


    for(auto& proc : all_processes)
    {
        cout<<"Process "<<proc->proc_no<<" Turnaround time: "<<proc->turn_around_time<<endl;
        average_turnaround_time+= proc->turn_around_time;

    }

    double size = all_processes.size();
    average_turnaround_time/= size;

    cout<<"Time quanta used: "<<time_quanta<<endl;
    cout<<"\nAverage Turnaround time: "<<average_turnaround_time<<endl;
    cout << "Total time taken: " << duration.count() << " microseconds" << endl;
    
    
    cout<<"Is queue empty: "<<process_queue.empty()<<endl;
    cout<<"Is waiting empty: "<<waiting_processes.empty()<<endl;
    if(!process_queue.empty())cout<<process_queue.front()->proc_no<<endl;
    if(curr_process)cout<<curr_process->proc_no<<endl;
    cout<<"Is current process null: "<<(curr_process==nullptr)<<endl;


    ofstream gantt_file("gantt_chart_RR.txt");
    gantt_file<<"CPU0"<<endl;

    for(auto& stat: gantt_chart)
    {
        gantt_file<<"P"<<stat[0]<<","<<stat[1]<<"\t\t"<<stat[2]<<"\t"<<stat[3]<<endl;
    }

    gantt_file.close();



    return 0;



}