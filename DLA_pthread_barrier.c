/*
	Diffusion-limited aggregation
	Pthread implementation with read/write lock

	gcc -Wall DLA_pthread_barrier.c -o run_pthread_b -lpthread   
	
	./run_pthread_b 100 100 10000 1500 50 50 2	
	./run_pthread_b 1000 1000 100000 100000 500 500 2

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/times.h>


int thread_count;   
pthread_rwlock_t rwlock;
char **griglia;
int height;
int width;
int iterazioni;
int numero_particelle;
struct coordinata *lista_particelle_da_cristallizare;
pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
struct coordinata{
	int x;
    int y;
};

int MAX(int a , int b);
int MIN(int a , int b);
void grid_to_img();
int controllo_cristalli(struct coordinata particella);
void * matrix_handler(void *params);

int main(int argc, char* argv[]) 
{
    struct timeval stop, start;
	  gettimeofday(&start, NULL);
	
    height = atoi(argv[2]);
    width = atoi(argv[1]);
    iterazioni = atoi(argv[3]);
    numero_particelle = atoi(argv[4]);
    int coordinate_cristallo_iniziale_x = atoi(argv[5]);
    int coordinate_cristallo_iniziale_y = atoi(argv[6]);
    thread_count = atoi(argv[7]); 

	griglia = (char**) malloc(sizeof(char *) * height);
    for(int i = 0; i < height; i++){
		    griglia[i] = (char *)malloc(sizeof(char) * width);
    }
	  for (int i = 0;i < height; i++){
		    for(int j = 0; j < width; j++){
			      griglia[i][j] = ' ';
        }
    }
	pthread_barrier_init(&barrier1, NULL, thread_count);
	pthread_barrier_init(&barrier2, NULL, thread_count);
	lista_particelle_da_cristallizare = malloc(sizeof(struct coordinata) * numero_particelle);
	
    //creo e lancio i vari thread che lavorano sulla funzione "matrix_handler" passandogli come unico parametro il proprio id
  	pthread_t* thread_handles;
    griglia[coordinate_cristallo_iniziale_x][coordinate_cristallo_iniziale_y] = 'o';
    
  	thread_handles = malloc (thread_count * sizeof(pthread_t));
  	long thread;
    pthread_rwlock_init(&rwlock, NULL);
  	for ( thread = 0; thread < thread_count; thread++){	
       	pthread_create(&thread_handles[thread], NULL, matrix_handler, (void *) thread);
    }

    for ( thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }
  
    griglia[coordinate_cristallo_iniziale_x][coordinate_cristallo_iniziale_y] = '}';
    grid_to_img();
	  gettimeofday(&stop, NULL);

    for(int i = 0; i < height; i++){
		    free(griglia[i]);
    }
    free(griglia);
    free(thread_handles);
	free(lista_particelle_da_cristallizare);
	//printf("execution time:%lu s\n", (stop.tv_sec - start.tv_sec)); 
	printf("execution time:%lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);   
  	return 0;
}

int MAX(int a , int b){
    return (a > b) ? a : b;
}

int MIN(int a , int b){
	  return (a > b) ? b : a;
}
void grid_to_img(){
    FILE *ppm_file = fopen("DLA_pthreadBarrier.ppm", "w");
    fprintf(ppm_file, "P3\n");
    fprintf(ppm_file, " \n");
    fprintf(ppm_file, "%d %d\n", width, height);
    fprintf(ppm_file, "255\n");
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(ppm_file, "%i ", (int)griglia[i][j]);
            fprintf(ppm_file, "%i ", (int)griglia[i][j]);
            fprintf(ppm_file, "%i   ", (int)griglia[i][j]);
        }
        fprintf(ppm_file, "\n");
    }
    fclose(ppm_file);
}

int controllo_cristalli(struct coordinata particella){
	  if(griglia[MAX(particella.x - 1, 0)][MAX(particella.y - 1, 0)] == 'o')                              // 1°caso
      return 1;
    else if(griglia[MAX(particella.x-1,0)][particella.y] == 'o')                                        // 2° caso
    	return 1;
    else if(griglia[MAX(particella.x -1, 0)][MIN(particella.y + 1, width-1)] == 'o')                    // 3° caso
    	return 1;
    else if(griglia[MIN(particella.x + 1,height-1)][MAX(particella.y -1,0)] == 'o')                     // 7°caso
    	return 1;
    else if(griglia[MIN(particella.x + 1,height-1)][particella.y] == 'o')                               // 8° caso
    	return 1;
    else if(griglia[MIN(particella.x + 1, height - 1)][MIN(particella.y + 1, width - 1)] == 'o')        // 9°caso
    	return 1;
    else if(griglia[particella.x][MAX(particella.y - 1, 0)] == 'o')                                     // 4° caso
      return 1;
    else if(griglia[particella.x][MIN(particella.y + 1, width - 1)] == 'o')                             // 6° caso
  		return 1;
	  else
      return 0;
}


void * matrix_handler(void *params)
{
    /*
    i vari thread simulano una parte delle particelle
    */
    long my_rank = (long)params;
  	unsigned int myseed = time(NULL) ^ getpid() ^ pthread_self();
  	int my_numero_particelle = (int)(numero_particelle / thread_count);
	int igs = (int)(numero_particelle /(thread_count ))*(my_rank); 
  	if (my_rank == 0)
    	  my_numero_particelle += numero_particelle % thread_count;
  
    struct coordinata my_lista_particelle[my_numero_particelle];
    for(int i = 0; i < my_numero_particelle; i++)
    {
        int x =  rand_r(&myseed) % height;
        int y =  rand_r(&myseed) % width;
        struct coordinata coordinata_particella = {x, y};
        my_lista_particelle[i] = coordinata_particella;
    }
	int n_particelle_cristallizare = 0;
  	for(int j = 0; j < iterazioni ; j++)
    {
		// SPOSTO TUTTE LE PARTICELLE
        for(int i = 0; i < my_numero_particelle; i++)
        {
			
            if(my_lista_particelle[i].x == 0)
                my_lista_particelle[i].x = my_lista_particelle[i].x + (rand_r(&myseed) % 2);		//rand( 0, 1);
            else if(my_lista_particelle[i].x == height - 1)
                my_lista_particelle[i].x = my_lista_particelle[i].x +  (rand_r(&myseed) % 2) - 1; //rand(-1, 0)
            else
      	        my_lista_particelle[i].x = my_lista_particelle[i].x + (1 - rand_r(&myseed) % 3);  //rand(-1, 0, 1)

            if(my_lista_particelle[i].y == 0)
                my_lista_particelle[i].y = my_lista_particelle[i].y + rand_r(&myseed) % 2; //rand( 0, 1);
            else if(my_lista_particelle[i].y == width - 1)
                my_lista_particelle[i].y = my_lista_particelle[i].y +  (rand_r(&myseed) % 2) - 1; //rand(-1, 0)
            else
      	        my_lista_particelle[i].y = my_lista_particelle[i].y + (1 - rand_r(&myseed) % 3); //rand(-1, 0, 1)

         
        }
		
		// ESEGUO IL CONTROLLO-CRISTALLI
		for(int i = 0; i < my_numero_particelle; i++)
        {
			 if(controllo_cristalli(my_lista_particelle[i]))
            {
				
              
				lista_particelle_da_cristallizare[n_particelle_cristallizare+igs] = my_lista_particelle[i];
                my_lista_particelle[i] = my_lista_particelle[my_numero_particelle - 1];
                my_numero_particelle--;
				
				n_particelle_cristallizare++;
            }
		}
	    pthread_barrier_wait(&barrier1);
		// AGGIORNO LA GRIGLIA 
		int i = 0;
		while(i < n_particelle_cristallizare )
		{
			
			griglia[ lista_particelle_da_cristallizare[i+igs].x ][ lista_particelle_da_cristallizare[i+igs].y ] = 'o';
			i++;
		}
		pthread_barrier_wait(&barrier2);	
		n_particelle_cristallizare = 0;
	
		

    }
	return NULL;
}
