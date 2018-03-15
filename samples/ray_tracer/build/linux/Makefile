all: clean raytracer

raytracer: main ray_tracer system_jobs math_matrix math_polygon image_image image_fourier image_main
	g++ \
	-L../../../dependencies/FreeImage/linux/ \
	-L../../../dependencies/squish-1.10/linux/ \
	main.o ray_tracer.o system_jobs.o math_matrix.o math_polygon.o image_image.o image_fourier.o image_main.o -lfreeimage -lsquish -lpthread -o raytracer
	
main: ../src/main.cpp
	g++ -g \
	-I../include/ \
	-I../../../include/ \
	-c ../src/main.cpp -o main.o

ray_tracer: ../src/ray_tracer.cpp
	g++ -g \
	-I../include/ \
	-I../../../include/ \
	-c ../src/ray_tracer.cpp -o ray_tracer.o

system_jobs: ../../../src/system/jobs.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/system/jobs.cpp -o system_jobs.o

math_matrix: ../../../src/math/matrix.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/math/matrix.cpp -o math_matrix.o
	
math_polygon: ../../../src/math/polygon.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/math/polygon.cpp -o math_polygon.o
	
image_image: ../../../src/image/image.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/linux/ \
	-I../../../dependencies/squish-1.10/linux/ \
	-c ../../../src/image/image.cpp -o image_image.o
	
image_fourier: ../../../src/image/fourier.cpp
	g++ -g \
	-I../../../include/ \
	-c ../../../src/image/fourier.cpp -o image_fourier.o
		
image_main: ../../../src/image/main.cpp
	g++ -g \
	-I../../../include/ \
	-I../../../dependencies/FreeImage/linux/ \
	-c ../../../src/image/main.cpp -o image_main.o
	
clean:
	rm -f *.o raytracer
