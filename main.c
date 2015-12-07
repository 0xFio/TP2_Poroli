/* ------------------------------------------
 * Project: La Banque de Grütschli / main.c
 * Authors: Poroli Fiorenzo
 * Version: 1.0, 2015-12-06
 * ------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "bank/bank.h"

#define ARGC_MIN 3

int main(int argc, char **argv) {

	if(argc < ARGC_MIN) {
		erreur_input_arg();
		return EXIT_SUCCESS;
	}
	uint nbr_people = atoi(argv[1]);
	uint val_d1 = atoi(argv[2]);
	double val_p = atof(argv[3]); 

	if (val_p > 1.0) {
		erreur_input_arg();
		return EXIT_SUCCESS;
	}		

	ticket_tot = 0;


	file_t *file = malloc(sizeof(*file));
	if (file == NULL) {
		printf("Erreur dans la création de la file ! \n");
		return EXIT_SUCCESS;
	}
	banker_t *banker = malloc(sizeof(banker_t));
	if (init_banker(banker, file, val_d1, val_p)) {
		printf("Erreur dans l'initialisation du banquier ! \n");
		return EXIT_SUCCESS;
	}

	// Create threads
	pthread_t* threads_customer = malloc(sizeof(pthread_t) * nbr_people);
	pthread_t* threads_banker = malloc(sizeof(pthread_t));

	if(pthread_create(threads_banker, NULL, banker_threads, banker)) {
		printf("Erreur dans la création du thread banker\n"); 
		return EXIT_FAILURE;
	}

	for(int i = 0; i < nbr_people; i++) {
		if(pthread_create(&threads_customer[i], NULL, customer_threads, banker)) {
			printf("Erreur dans la création du thread customeeerrr\n"); 
			return EXIT_FAILURE;
		}
	}

	pthread_join(threads_banker, NULL); // infini car il ne s'arrete pas ...
	
	return EXIT_SUCCESS;
}
