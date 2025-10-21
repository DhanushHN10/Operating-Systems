part1: run-sharpen clean-part1

./part1.out: part1/image_sharpener.cpp part1/libppm.cpp
	g++ -g part1/image_sharpener.cpp part1/libppm.cpp -o part1/part1.out

run-sharpen: ./part1.out
	part1/part1.out image.ppm part1/output_part1.ppm

clean-part1:
	rm part1/part1.out
