/* ------------------------------------------
 * Project: TP2 La banque de Grütschli / bank.c
 * Authors: Poroli Fiorenzo
 * Version: 1.0, 2015-12-06
 * ------------------------------------------*/

#ifndef __BANQUE_H__
#define __BANQUE_H__

#include "bank.h"

/** 
 * @brief Fonction pour l'initialisation de la banque
 * @param banker_t* b le Banquier
 * @param file_t* f la file pour le FiFo
 * @param int d1 attentes passives pour le sleep
 * @param double p probabilité pour le client de rester dans la banque
 */
bool init_banker(banker_t* b, file_t* f, int d1, double p) {

	if (b == NULL)
		return true;

	b->ticket_served = 0;

	if (sem_init(&b->waiting_room, 0, 0)) {
		printf("erroorr");
	}


	b->file = f;
	b->file->first_customer_served = NULL;

	if (sem_init(&b->file->sem_file, 0, 1)) {
		printf("error initialisation du banquier");
	}
	//b->file->test = 120;

	//b->customer = NULL;
	b->p = p;
	b->d1 = d1;
	b->d0 = 10*d1;
	
	return false;
}


/** 
 * @brief Fonction pour ajouter un client et initialiser le client
 * @param file_t* file la file pour le FiFo
 */
customer_t* add_customer(file_t *file) {

	customer_t *new_customer = malloc(sizeof(customer_t));
	if (file == NULL || new_customer == NULL) {
		printf("Erreur dans l'ajout d'un client\n");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&new_customer->be_served, 0, 0)) {	// init du semafor pour etre servie
		printf("Erreur Semafor du client\n");
		exit(EXIT_FAILURE);
	}

	new_customer->my_place = BANK;

	
//printf("%d --- ", file->test);

	sem_wait(&file->sem_file);

	pthread_mutex_lock(&mu_ticket_tot);	// prendre le dernier ticket
	ticket_tot++;
	new_customer->my_ticket = ticket_tot;
	pthread_mutex_unlock(&mu_ticket_tot);

	pthread_mutex_lock(&file->mu_file);		// Se placer en dernier de la file
	if (file->first_customer_served !=NULL) {
		
		customer_t *scan_customer = file->first_customer_served;
		while (scan_customer->next_customer != NULL) {
			scan_customer = scan_customer->next_customer;
		}
		scan_customer->next_customer = new_customer;
		new_customer->next_customer = NULL;
	}
	else {
		file->first_customer_served = new_customer;
		new_customer->next_customer = NULL; //file->first_customer_served-
	}
	pthread_mutex_unlock(&file->mu_file);

	sem_post(&file->sem_file);


	return new_customer;
}


/** 
 * @brief Fonction de thread pour le client
 * @param void* b structure du banquier
 */
void* customer_threads(void* b) {

	uint activity = 0; 	// pour l'activité d1*(my_ticket - t_servie)
	uint my_ticket = 0;

	banker_t* banker = (banker_t*) b;
	customer_t *customer;
	//customer_t* customer = (customer_t*) banker->customer; 

	while(true) {
		take_new_ticket:	// !! Retour du GOTO !!

		sleep(banker->d0);
		customer = add_customer(banker->file);

		pthread_mutex_lock(&banker->mu_ticket_served);
		if ((customer->my_ticket - banker->ticket_served) > 3) {
			pthread_mutex_unlock(&banker->mu_ticket_served);

			if (random_n() < banker->p) {
		
				my_ticket = customer->my_ticket;

			//	printf("  %d : Je suis sortie un moment \n", customer->my_ticket);
				customer->my_place = OUTSIDE;
				pthread_mutex_lock(&banker->mu_ticket_served);
				activity = customer->my_ticket - banker->ticket_served;
				pthread_mutex_unlock(&banker->mu_ticket_served);
				sleep(banker->d1*activity);
				// le client revient à la banque contrôler si son ticket est passé
				pthread_mutex_lock(&customer->mu_my_place);
				//if (
				customer->my_place = BANK;
				pthread_mutex_unlock(&customer->mu_my_place);


				pthread_mutex_lock(&banker->mu_ticket_served);
				if (banker->ticket_served > my_ticket) {			// un problème dans cette zone !!!
					pthread_mutex_unlock(&banker->mu_ticket_served);
					printf("  Mince je dois prendre un nouveau ticket\n");
					free(customer);
					goto take_new_ticket;
				}
			}
		}
		pthread_mutex_unlock(&banker->mu_ticket_served);
		sem_post(&banker->waiting_room);
		sem_wait(&customer->be_served);
		free(customer);
	}
}


/** 
 * @brief Fonction pour servir le premier client de la file
 * @param banker_t* b structure du banquier
 */
void served_first_customer(banker_t *b) {

	banker_t* banker = (banker_t*) b;
	customer_t *customer_served;

	customer_served = banker->file->first_customer_served;
	
	printf("n° %d servie  \n", customer_served->my_ticket); 
	sleep(banker->d1);
	delete_first_customer(banker->file);
	sem_post(&customer_served->be_served);

}


/** 
 * @brief Fonction pour detruire le ticket et de la file un client
 * @param file_t *file file pour le FiFo 
 */
void delete_first_customer(file_t *file) {

	if (file == NULL) {
		printf("Erreur dans le served customer\n");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_lock(&file->mu_file);
	if (file->first_customer_served != NULL) {
		customer_t *customer_served = file->first_customer_served;
		file->first_customer_served = customer_served->next_customer;
		//free(customer_served);
	}
	pthread_mutex_unlock(&file->mu_file);
}



/** 
 * @brief Fonction de thread du banquier
 * @param void* b structure du banquier 
 */
void* banker_threads(void* b) {

	banker_t* banker = (banker_t*) b;

	while(true) {

		sem_wait(&banker->waiting_room);

		pthread_mutex_lock(&banker->mu_ticket_served);
		banker->ticket_served++;
		pthread_mutex_unlock(&banker->mu_ticket_served);

		pthread_mutex_lock(&mu_ticket_tot);
		if (banker->ticket_served <= ticket_tot) { // verifiaction si il y a un ou des clients
			pthread_mutex_unlock(&mu_ticket_tot);			
			pthread_mutex_lock(&banker->file->first_customer_served->mu_my_place);
			if (banker->file->first_customer_served->my_place == BANK) { // verifie si le client est à la banque 
				pthread_mutex_unlock(&banker->file->first_customer_served->mu_my_place);
				served_first_customer(banker);// servir le client
			}
			else { // client absent de la banque
				pthread_mutex_unlock(&banker->file->first_customer_served->mu_my_place);
				printf("Ticket n°%d n'est pas à la banque\n", banker->file->first_customer_served->my_ticket);
				delete_first_customer(banker->file);
			}
		}
		pthread_mutex_unlock(&mu_ticket_tot);
	}
}


/** 
 * @brief Fonction pour un nombre aléatoire entre0 et 1
 * @return double valeur entre 0 et 1 aléatoire 
 */
double random_n() {

	srand(time(NULL));
	return (double)rand() / RAND_MAX;
}


/** 
 * @brief Fonction erreur dans l'écriture + explication.
 */
void erreur_input_arg() {

	printf("Erreur d'écriture : \n");
	printf("./townGrutschli nbr_people val_d1 val_p \n");
	printf("nbr_people = population (int) / minimum 1\n");
	printf("val_d1 = valeur d'attentes passives (int) / minimum 1\n");
	printf("val_p = valeur de probabilité pour rester dans la banque / double entre 0 et 1\n");
	printf("\n");
}


#endif
