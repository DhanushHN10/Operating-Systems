buildall: FIFO_OPTIMAL.out LRU_and_Random.out memoryManagement.out

FIFO_OPTIMAL.out: FIFO_OPTIMAL.cpp
	g++ -o FIFO_OPTIMAL.out FIFO_OPTIMAL.cpp
LRU_and_Random.out: LRU_and_Random.cpp
	g++ -o LRU_and_Random.out LRU_and_Random.cpp
memoryManagement.out: memoryManagement.cpp
	g++ -o memoryManagement.out memoryManagement.cpp
clean:
	rm -f *.out results.txt