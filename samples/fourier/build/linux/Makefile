all: clean fourier

ifeq ($(OS), Windows_NT)
fourier: main math_matrix math_polygon image_image image_fourier image_main
	g++ \
	-L../../../dependencies/FreeImage/windows/ \
	-L../../../dependencies/squish-1.10/windows/ \
	main.o math_matrix.o math_polygon.o image_image.o image_fourier.o image_main.o -lfreeimage -lsquish -o fourier
else
fourier: main math_matrix math_polygon image_image image_fourier image_main
	g++ \
	-L../../../dependencies/FreeImage/linux/ \
	-L../../../dependencies/squish-1.10/linux/ \
	main.o math_matrix.o math_polygon.o image_image.o image_fourier.o image_main.o -lfreeimage -lsquish -o fourier
endif

main: ../src/main.cpp
	g++ -g \
	-I../../../include/ \
	-c ../src/main.cpp -o main.o

math_matrix: ../../../src/math/matrix.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/math/matrix.cpp -o math_matrix.o
	
math_polygon: ../../../src/math/polygon.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/math/polygon.cpp -o math_polygon.o
	
ifeq ($(OS), Windows_NT)
image_image: ../../../src/image/image.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/windows/ \
	-I../../../dependencies/squish-1.10/windows/ \
	-c ../../../src/image/image.cpp -o image_image.o
else
image_image: ../../../src/image/image.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/linux/ \
	-I../../../dependencies/squish-1.10/linux/ \
	-c ../../../src/image/image.cpp -o image_image.o
endif
	
image_fourier: ../../../src/image/fourier.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/image/fourier.cpp -o image_fourier.o
	
ifeq ($(OS), Windows_NT)	
image_main: ../../../src/image/main.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/windows/ \
	-c ../../../src/image/main.cpp -o image_main.o
else
image_main: ../../../src/image/main.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/linux/ \
	-c ../../../src/image/main.cpp -o image_main.o
endif
	
clean:
	rm *.o fourier
