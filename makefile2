part1: run-sharpen clean-part1

./part1.out: part1/image_sharpener.cpp part1/libppm.cpp
	g++ -g part1/image_sharpener.cpp part1/libppm.cpp -o part1/part1.out

run-sharpen: ./part1.out
	part1/part1.out image.ppm part1/output_part1.ppm

clean-part1:
	rm part1/part1.out

part2_1: run-sharpen_part2_1 clean-part2_1

./part2_1.out: part2/image_sharpener_part2_1.cpp part2/libppm.cpp
	g++ -g part2/image_sharpener_part2_1.cpp part2/libppm.cpp -o part2/part2_1.out

run-sharpen_part2_1: ./part2_1.out
	part2/part2_1.out image.ppm part2/output_part2_1.ppm

clean-part2_1:
	rm part2/part2_1.out