input: 
	width, height, numero_particelle, iterazioni, coordinate_cristallo_iniziale_x, coordinate_cristallo_iniziale_y, numero_di_threads*

Serial:

	gcc -o run_serial DLA_serial_def.c
	
	./run_serial 100 100 10000 1500 50 50 
	./run_serial 1000 1000 100000 100000 500 500 

OpenMP :
	
	gcc -g -Wall -fopenmp -o run_omp DLA_omp_def.c
	
	./run_omp 100 100 10000 1500 50 50 2	
	./run_omp 1000 1000 100000 100000 500 500 2
	
Pthread read/write lock:

	gcc -Wall DLA_pthreadRWLock_def.c -o run_pthread_rw -lpthread   
	
	./run_pthread_rw 100 100 10000 1500 50 50 2	
	./run_pthread_rw 1000 1000 100000 100000 500 500 2

Pthread barrier:

	gcc -Wall DLA_pthread_barrier.c -o run_pthread_b -lpthread   
	
	./run_pthread_b 100 100 10000 1500 50 50 2	
	./run_pthread_b 1000 1000 100000 100000 500 500 2