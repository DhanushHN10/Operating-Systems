CXX = g++
CXXFLAGS = -std=c++11 -pthread -g

INPUT_IMAGE = image.ppm	
ITER = 1000

part1:
	cd part1 && $(CXX) $(CXXFLAGS) image_sharpener.cpp libppm.cpp -o 1.out
	cd part1 && ./1.out ../$(INPUT_IMAGE) output_part1.ppm $(ITER)

part2_1:
	cd part2 && $(CXX) $(CXXFLAGS) image_sharpener_part2_1.cpp libppm.cpp -o 2_1.out
	cd part2 && ./2_1.out ../$(INPUT_IMAGE) output_part2_1.ppm 

part2_2:
	cd part2 && $(CXX) $(CXXFLAGS) image_sharpener_part2_2.cpp libppm.cpp -o 2_2.out
	cd part2 && ./2_2.out ../$(INPUT_IMAGE) output_part2_2.ppm $(ITER)

part2_3:
	cd part2 && $(CXX) $(CXXFLAGS) image_sharpener_part2_3.cpp libppm.cpp -o 2_3.out
	cd part2 && ./2_3.out ../$(INPUT_IMAGE) output_part2_3.ppm $(ITER)

part3_1_A:
	cd part3 && $(CXX) $(CXXFLAGS) image_sharpener_part3_1_A.cpp libppm.cpp -o 3_1_A.out
	cd part3 && ./3_1_A.out ../$(INPUT_IMAGE)  5

part3_1_B:
	cd part3 && $(CXX) $(CXXFLAGS) image_sharpener_part3_1_B.cpp libppm.cpp -o 3_1_B.out
	cd part3 && ./3_1_B.out ../$(INPUT_IMAGE) output_part3_1.ppm 5

part3_2_A:
	cd part3 && $(CXX) $(CXXFLAGS) image_sharpener_part3_2_A.cpp libppm.cpp -o 3_2_A.out
	cd part3 && ./3_2_A.out ../$(INPUT_IMAGE) 5

part3_2_B:
	cd part3 && $(CXX) $(CXXFLAGS) image_sharpener_part3_2_B.cpp libppm.cpp -o 3_2_B.out
	cd part3 && ./3_2_B.out ../$(INPUT_IMAGE) output_part3_2.ppm 5

clean:
	find . -type f -name "*.out" -delete