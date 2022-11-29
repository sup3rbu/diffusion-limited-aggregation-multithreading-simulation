/*
Diffusion-limited aggregation (DLA) è un processo di formazione di cristalli nel quale le particelle si
muovono in uno spazio 2D con moto browniano (cioè in modo casuale) e si combinano tra loro  quando si toccano.
DLA può essere simulato utilizzando una griglia 2D in cui ogni cella può essere  occupata da uno o più particelle in movimento.
Una particella diventa parte di un cristallo (e si  ferma) quando si trova in prossimità di un cristallo già formato.
I parametri di base della simulazione  sono la dimensione della griglia 2D,
il numero iniziale di particelle, il numero di iterazioni e il "seme"  cristallino iniziale. Realizzare sia un’implementazione che utilizza OpenMP, sia un’implementazione  che utilizza una Pthread.



input: width, height, numero_particelle, iterazioni, coordinate_cristallo_iniziale_x, coordinate_cristallo_iniziale_y

gcc -o run_serial DLA_serial_def.c
./run_serial 100 100 10000 1500 50 50 
./run_serial 1000 1000 100000 100000 500 500 

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>

//struttura che definisce una coordinata, con parametri le coordinate di riga e colonna.
struct coordinata{
	int x;
    int y;
};

int MAX(int a , int b){
  return (a > b) ? a : b;
}

int MIN(int a , int b){
	return (a > b) ? b : a;
}



//grid_to_img trasforma la matrice ottenuta in un file in formato ppm che ci permette visualizzare a schermo il risultato finale ottenuto
void grid_to_img(int width, int height, char griglia[height][width]){
    FILE *ppm_file = fopen("DLA_serial.ppm", "w");
    fprintf(ppm_file, "P3\n");
    fprintf(ppm_file, " \n");
    fprintf(ppm_file, "%d %d\n", width, height);
    fprintf(ppm_file, "255\n");

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(ppm_file, "%i %i %i   ", (int)griglia[i][j], (int)griglia[i][j], (int)griglia[i][j]);
        }
        fprintf(ppm_file, "\n");
    }
    fclose(ppm_file);
}

//funzione che controlla se nei dintorni della particella è presente un cristallo controllando se nella matrice è presente un carattere 'o'
int controllo_cristalli(struct coordinata particella, int width, int height, char griglia[height][width]){
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

/*

La matrice di appoggio utilizzata è denotata dalla presenza di caratteri che indicano:
 "o" = particella cristallizzata
 " " = cella senza cristallo
 "}" = cristallo iniziale
*/

int main(int argc, char *argv[]){

    //calcolo del tempo di esecuzione del programma
    struct timeval stop, start;                                             
	gettimeofday(&start, NULL);
    
    //prendo i parametri da linea di comando
	int height = atoi(argv[2]);
    int width = atoi(argv[1]);
    int iterazioni = atoi(argv[3]);
    int numero_particelle = atoi(argv[4]);
    int coordinate_cristallo_iniziale_x = atoi(argv[5]);
    int coordinate_cristallo_iniziale_y = atoi(argv[6]);

    //inizializzo la griglia
    char griglia[height][width];
	for (int i = 0; i < height; i++)
		for(int j = 0; j < width; j++)
				griglia[i][j] = ' ';

    griglia[coordinate_cristallo_iniziale_x][coordinate_cristallo_iniziale_y] = 'o';
    struct coordinata lista_particelle[numero_particelle];
    
    //genero le varie particelle assegnandole in una posizione casuale nella griglia
    srand(time(0));
    for(int i = 0; i < numero_particelle; i++)
    {
        int x =  rand() % height;
        int y =  rand() % width;
        struct coordinata coordinata_particella = {x, y};
        lista_particelle[i] = coordinata_particella;
    }

    //inizio della simulazione
    for(int j = 0;j < iterazioni; j++)
    {
        for(int i = 0; i < numero_particelle; i++)
        {
            /* 
            simuliamo il moto browniano di una particella , generando randomicamente un numero tra -1,0,1 che andremo a sommare alle coordinate correnti 
            della particella dando attenzione alla posizione della particella nella griglia
            */
            if(lista_particelle[i].x == 0)
                lista_particelle[i].x = lista_particelle[i].x + (rand() % 2);		        //rand( 0, 1);
            else if(lista_particelle[i].x == height - 1)
                lista_particelle[i].x = lista_particelle[i].x +  (rand() % 2) - 1;          //rand(-1, 0)
            else
      	        lista_particelle[i].x = lista_particelle[i].x + (1 - rand() % 3);           //rand(-1, 0, 1)
            if(lista_particelle[i].y == 0)
                lista_particelle[i].y = lista_particelle[i].y + rand() % 2;                 //rand( 0, 1);
            else if(lista_particelle[i].y == width - 1)
                lista_particelle[i].y = lista_particelle[i].y +  (rand() % 2) - 1;          //rand(-1, 0)
            else
      	        lista_particelle[i].y = lista_particelle[i].y + (1 - rand() % 3);           //rand(-1, 0, 1)

            //controllo se nei dintorni della particella è presente un cristallo e nel caso essa si trasformerà in cristallo
            if(controllo_cristalli(lista_particelle[i], width, height, griglia))
            {
                griglia[lista_particelle[i].x][lista_particelle[i].y] = 'o';
                lista_particelle[i] = lista_particelle[numero_particelle - 1];
                numero_particelle--;
            }
        }
    }

	griglia[coordinate_cristallo_iniziale_x][coordinate_cristallo_iniziale_y] = '}';
    grid_to_img(width, height, griglia);

    gettimeofday(&stop, NULL);
  	//printf("execution time:  %lu s\n", (stop.tv_sec - start.tv_sec) );
	printf("execution time:  %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
	return 0;
}