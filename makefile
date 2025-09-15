buildall: npsjf.out psjf.out npsjf_multi.out psjf_multi.out fifo.out fifo_dual.out rr.out rr_dual.out all.out all_multi.out

npsjf.out: npsjf.cpp
	g++ npsjf.cpp -o npsjf.out

psjf.out: psjf.cpp
	g++ psjf.cpp -o psjf.out

npsjf_multi.out: npsjf_multi.cpp
	g++ npsjf_multi.cpp -o npsjf_multi.out
psjf_multi.out: psjf_multi.cpp
	g++ psjf_multi.cpp -o psjf_multi.out
fifo.out: fifo_algorithm.cpp
	g++ fifo_algorithm.cpp -o fifo.out
fifo_dual.out: fifo_algorithm_dual_core.cpp
	g++ fifo_algorithm_dual_core.cpp -o fifo_dual.out
rr.out: roundRobin_algorithm.cpp
	g++ roundRobin_algorithm.cpp -o rr.out
rr_dual.out: roundRobin_algorithm_multi_core.cpp
	g++ roundRobin_algorithm_multi_core.cpp -o rr_dual.out
all.out: all_single_core.cpp
	g++ all_single_core.cpp -o all.out
all_multi.out: all_multi_core.cpp
	g++ all_multi_core.cpp -o all_multi.out

clean:
	rm -f *.out
