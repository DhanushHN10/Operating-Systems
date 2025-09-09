#include<iostream>
#include<string>
#include<vector>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include<queue>
#include <sstream>
using namespace std;


class Process{
    private:
    bool cpuBurst=true;

    

    public:

    int arrival_time;
    vector<int> cpu_burst_times;
    vector<int> io_times;
    //io_times.push_back(0);
    string state = "ready";
    int proc_no;

    int turn_around_time;

    Process(int arr_time)
    {
        io_times.push_back(0);
        arrival_time = arr_time;
    }

    int add_cpu_time(int cpu_time)
    {
        cpu_burst_times.push_back(cpu_time);
        cout<<"Added cput"<<cpu_time<<endl;
        return 0;
    }
    int add_IO_time(int IO_time)
    {
        io_times.push_back(IO_time);
        cout<<"Added iot"<<IO_time<<endl;
        return 0;
    }

};

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout<<"you dumbeo enter 2\n";
    }

    queue<Process> process_queue;

    ifstream file(argv[1]);
    string line;
    // getline(file, s);
    // getline(file, s);
    // getline(file, s);
    //file.seekg(0);
    // int xx;
    // while(file >> xx)
    // {
    //     cout<<"read "<<xx<<endl;
    // }
    getline(file, line);
    while(line[0] == '<')
    {
        getline(file, line);
    }
    long long time = 0;
    while(1)
    {
        int arrival_time;
        stringstream ss;
        ss << line;
        if(line[0] == '<')
            break;

        
        ss >> arrival_time;
        cout<<arrival_time<<endl;
        if(arrival_time == time)
        {
            Process np(time);
            int x;
            bool cpu_time = true;
            ss >> x;
            //cout<<" "<<x<<" ";
            while(x != -1)
            {
                if(cpu_time)
                {
                    np.add_cpu_time(x);
                    cpu_time = !cpu_time;
                }
                else
                {
                    np.add_IO_time(x);
                    cpu_time = !cpu_time;
                }
                //break;
                ss >> x;
            }
            process_queue.push(np);
            getline(file, line);
        }
        
        time++;
        //break;
    }

    

}
