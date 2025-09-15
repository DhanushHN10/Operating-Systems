# Operating-Systems
Repository for Operating Systems Lab Assignments

Lab Assignment - 3


# Steps to run on single core system
Run:  
make clean  
make buildall  
./all.out <data_file> <algorithm> <time_quanta>  

# Steps to run on dual core system  
Run:  
make clean  
make buildall  
./all_multi.out <data_file> <algorithm> <time_quanta>  


# Algorithms:  
"fifo":first in first out  
"npsjf":non preemptive shortest job first  
"psjf":preemptive shortest job first  
"rr": round-robin  

# Example:  
./all.out process1.dat npsjf  



Note: time_quanta argument is only used for round-robin  

