#include<iostream>
#include<cstring>
#include<unistd.h>
#include<sys/wait.h>
using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout<<"Enter 2 arguments\n";
		exit(0);
	}
	
	
	{
		
		if(strcmp(argv[2],"npsjf") == 0){
			char* args1 = strdup("./npsjf");
			char* command1[] = {args1, argv[1], NULL};
			int pid1 = fork();
			if(pid1 == 0)
			{
				execvp(command1[0], command1);
			}
			else
			{
				wait(NULL);
			}
		}
		else if(strcmp(argv[2], "psjf") == 0){
			char* args2 = strdup("./psjf");
			char* command2[] = {args2, argv[1], NULL};
			int pid2 = fork();
			if(pid2 == 0)
			{
				execvp(command2[0], command2);
			}
			else
			{
				wait(NULL);
			}
		}	
		
	}
}
