#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "shmem.h"
#include "semaphore.h"


int fin = 0;
int descpipe[2];
char tabledst[10][20] = {"Paris","Toulouse","Lyon","Lille","Bordeaux","Nimes","Nantes","Marseille","Strasbourg","Nice"};


void fin1()
{
	fin = 1;
	close(descpipe[1]);
}

void fin2()
{
	fin = 1;
	close(descpipe[0]);
}

struct vol
{
	int nb_places;
	char dest[20];
};

typedef struct vol VOL;

 	

int main(void)
{
	srand(time(NULL));	

	if(pipe(descpipe)<0)
	{
		fprintf(stderr,"erreur création tube ");
		exit(1);
	}

	int pid_Tirage = fork();
	
	if(pid_Tirage > 0)
	{	
		close(descpipe[0]);
		char nom[10] = "Tirage : ";
		signal(SIGINT,fin1);
		
		printf("%s Je suis le tirage. \n", nom);
		
		VOL vol;
		printf("%s Je génére un vol.....   ", nom);
		vol.nb_places = rand()%5+1;
		strcpy(vol.dest, tabledst[rand()%10]);
		printf("Destination : %s || Nombre de place(s) : %d \n", vol.dest, vol.nb_places);
		write(descpipe[1],&vol,sizeof(VOL));
		//close(descpipe[1]);
		
	}






        else
	{	
		int init;
		close(descpipe[1]);
		char nom[20] = "Ecrivain : ";
		signal(SIGINT,fin2);
		printf("%s Je suis l'ecrivain. \n", nom);

    
    /* Création des sémaphores */


		printf("%s Je crée les sémaphores. \n", nom);
		int mutex;
		mutex = create_semaphore(10);
		if(mutex <0){
			fprintf(stderr, "erreur création mutex \n");
			exit(1);
		}		
		
		if((init = init_semaphore(mutex, 1) < 0)){
			fprintf(stderr, "erreur init mutex \n");
			exit(1);
		}

		int nb_vols;
		nb_vols = create_semaphore(11);
		if(nb_vols <0){
			fprintf(stderr, "erreur création nb_vols \n");
			exit(1);
		}		
		
		if((init = init_semaphore(nb_vols, 20) < 0)){
			fprintf(stderr, "erreur init nb_vols\n");
			exit(1);
		}


		printf("%s Les sémaphores ont été crées avec succès. \n", nom);
		
    /* Création de la mémoire partagée */

    int shm = create_shmem(100, 20*(sizeof(VOL)));
    if(shm<0){
      fprintf(stderr,"erreur création BDD \n");
    }
    VOL* bdd = (VOL*)attach_shmem(shm);
		



		VOL vol;
		printf("%s Je lis le vol.....  ", nom);
		while(read(descpipe[0],&vol,sizeof(VOL))){
			printf("%s Destination : %s || Nombre de place(s) : %d \n", nom, vol.dest, vol.nb_places);
		}
	  
    down(mutex);
    
    bdd[0] = vol;
    printf("%s J'ai écrit le vol dans la case 0 de la bdd. \n",nom);

    printf("%s %s , %d \n ", nom, bdd[0].dest, bdd[0].nb_places);
    
    up(mutex);

	}

	return(0);
}


