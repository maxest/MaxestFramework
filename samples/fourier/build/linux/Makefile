all:  fourier

fourier: main math_matrix math_polygon image_image image_conversion image_filtering image_fourier image_main
	g++ \
	-L../../../../dependencies/FreeImage-3.17.0/lib/ \
	-L../../../../dependencies/squish-1.11/lib/ \
	main.o math_matrix.o math_polygon.o image_image.o image_conversion.o image_filtering.o image_fourier.o image_main.o -lfreeimage -lsquish -o fourier

main: ../../src/main.cpp
	g++ -std=c++11 -g \
	-c ../../src/main.cpp -o main.o

math_matrix: ../../../../src/math/matrix.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/math/matrix.cpp -o math_matrix.o
	
math_polygon: ../../../../src/math/polygon.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/math/polygon.cpp -o math_polygon.o
	
image_image: ../../../../src/image/image.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/image/image.cpp -o image_image.o
	
image_conversion: ../../../../src/image/conversion.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/image/conversion.cpp -o image_conversion.o
	
image_filtering: ../../../../src/image/filtering.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/image/filtering.cpp -o image_filtering.o
	
image_fourier: ../../../../src/image/fourier.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/image/fourier.cpp -o image_fourier.o

image_main: ../../../../src/image/main.cpp
	g++ -std=c++11 -g \
	-c ../../../../src/image/main.cpp -o image_main.o
	
clean:
	rm *.o fourier
