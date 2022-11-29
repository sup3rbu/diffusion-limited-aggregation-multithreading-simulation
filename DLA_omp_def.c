/*
	Diffusion-limited aggregation
	openmp implementation	

	gcc -g -Wall -fopenmp -o run_omp DLA_omp_def.c
	
	./run_omp 100 100 10000 1500 50 50 2	
	./run_omp 1000 1000 100000 100000 500 500 2

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/times.h>
#include <omp.h>

struct coordinata{
	int x;
    int y;
};

int MAX(int a , int b){
  return (a > b ) ? a : b;
}

int MIN(int a , int b){
	return (a > b ) ? b : a;
}

//input: width, height, numero_particelle, iterazioni, coordinate_cristallo_iniziale_x, coordinate_cristallo_iniziale_y

//grid_to_img ci permette di trasformare la matrice ottenuta in un file in formato ppm che ci permette fotografare il risultato finale ottenuto
void grid_to_img(int width, int height, char griglia[height][width]){
    FILE *ppm_file = fopen("DLA-OpenMP.ppm", "w");
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


//controllo se nei dintorni della particella è presente un cristallo visualizzando se nella matrice è presente un carattere 'o'
int controllo_cristalli(struct coordinata particella, int width, int height, char griglia[height][width]){
	if(griglia[MAX(particella.x - 1, 0)][MAX(particella.y - 1, 0)] == 'o')                                    // 1°caso
    	return 1;
    else if(griglia[MAX(particella.x-1,0)][particella.y] == 'o')                                            // 2° caso
    	return 1;
    else if(griglia[MAX(particella.x -1, 0)][MIN(particella.y + 1, width-1)] == 'o')                        // 3° caso
    	return 1;
    else if(griglia[MIN(particella.x + 1,height-1)][MAX(particella.y -1,0)] == 'o')                         // 7°caso
    	return 1;
    else if(griglia[MIN(particella.x + 1,height-1)][particella.y] == 'o')                                   // 8° caso
    	return 1;
    else if(griglia[MIN(particella.x + 1, height - 1)][MIN(particella.y + 1, width - 1)] == 'o')            // 9°caso
    	return 1;
    else if(griglia[particella.x ][MAX(particella.y-1,0)] == 'o')                                           // 4° caso
      return 1;
    else if(griglia[particella.x][MIN(particella.y+1,width-1)] == 'o')                                      // 6° caso
  		return 1;
	  else
      return 0;

}

int main(int argc, char *argv[])
{
	
	struct timeval stop, start;
	gettimeofday(&start, NULL);
	int height = atoi(argv[2]);
    int width = atoi(argv[1]);
    int iterazioni = atoi(argv[3]);
    int numero_particelle = atoi(argv[4]);
    char griglia[height][width];
	for (int i = 0; i < height; i++)
		for(int j = 0; j < width; j++)
			griglia[i][j] = ' ';

    int coordinate_cristallo_iniziale_x = atoi(argv[5]);
    int coordinate_cristallo_iniziale_y = atoi(argv[6]);
  	int thread_count = atoi(argv[7]);
    griglia[coordinate_cristallo_iniziale_x][coordinate_cristallo_iniziale_y] = 'o';

    struct coordinata lista_particelle_da_cristallizare[numero_particelle];
	
    srand(time(0));
    //genero le varie particelle assegnandole in una posizione casuale nella griglia
    
    //eseguo il seguente blocco di codice in parallelo con (thread_count) threads
    #pragma omp parallel num_threads(thread_count)
	{
		/*
		i vari thread simulano una parte delle particelle
		*/
		int my_rank = omp_get_thread_num();
		int igs = (int)(numero_particelle /(thread_count ))*(my_rank); 
		int n_particelle_cristallizzare = 0;
		unsigned int myseed = time(NULL) ^ my_rank;
		int my_numero_particelle = (int)(numero_particelle / thread_count);
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
	  

		for(int j = 0; j < iterazioni ; j++)
		{
			for(int i = 0; i < my_numero_particelle; i++)
			{
				// simuliamo il moto browniano di una particella , generando randomicamente un numero tra -1,0,1 che andremo a sommare alle coordinate correnti della particella
				if(my_lista_particelle[i].x == 0)
					my_lista_particelle[i].x = my_lista_particelle[i].x + (rand_r(&myseed) % 2);		      //rand( 0, 1);
				else if(my_lista_particelle[i].x == height - 1)
					my_lista_particelle[i].x = my_lista_particelle[i].x +  (rand_r(&myseed) % 2) - 1;     //rand(-1, 0)
				else
					my_lista_particelle[i].x = my_lista_particelle[i].x + (1 - rand_r(&myseed) % 3);      //rand(-1, 0, 1)

				if(my_lista_particelle[i].y == 0)
					my_lista_particelle[i].y = my_lista_particelle[i].y + rand_r(&myseed) % 2;            //rand( 0, 1);
				else if(my_lista_particelle[i].y == width - 1)
					my_lista_particelle[i].y = my_lista_particelle[i].y +  (rand_r(&myseed) % 2) - 1;     //rand(-1, 0)
				else
					my_lista_particelle[i].y = my_lista_particelle[i].y + (1 - rand_r(&myseed) % 3);      //rand(-1, 0, 1)

				//controllo se nei dintorni della particella è presente un cristallo e nel caso essa si trasformerà in cristallo
			}	
	  
			//BARRIER
			#pragma omp barrier

			for(int i = 0; i < my_numero_particelle; i++)
			{
				if(controllo_cristalli(my_lista_particelle[i], width, height, griglia))
				{
					lista_particelle_da_cristallizare[n_particelle_cristallizzare+igs] = my_lista_particelle[i];
					my_lista_particelle[i] = my_lista_particelle[my_numero_particelle - 1];
					my_numero_particelle--;
					n_particelle_cristallizzare++;
				}
			}
			//PER OGNI PARTICELLA FACCIO IL CONTROLLO SE SI LEVO E INSERISCO NELLA LISTA APPOSITA VADO AVANTI
			//AGGIORNO GRIGLIA
			int i = 0;
			while(i < n_particelle_cristallizzare )
			{
				griglia[lista_particelle_da_cristallizare[i+igs].x ][ lista_particelle_da_cristallizare[i+igs].y ] = 'o';
				i++;
			}
			n_particelle_cristallizzare = 0;
			//BARRIER
			#pragma omp barrier
		}
	}
  
	grid_to_img(width, height, griglia);

	gettimeofday(&stop, NULL);
	
	printf(" execution time:%lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); 
	
	
	return 0;
}