/* ------------------------------------------
 * Project: TP2 La banque de Grutschli / bank.h
 * Authors: Poroli Fiorenzo
 * Version: 1.0 2015-12-06
 * ------------------------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define RANDOM_MAX 



/**
 * enum place
 */
typedef enum {
	BANK, 
	OUTSIDE,
} place;

/**
 * Structure customer
 */
typedef struct customer customer_t;
struct customer
{
	place my_place;
	uint my_ticket;
	customer_t *next_customer;
	sem_t be_served;
	
};

/**
 * Structure file (fifo)
 */
typedef struct file file_t;
struct file
{
	customer_t *first_customer_served;
	pthread_mutex_t mu_file;
};


/**
 * Structure banker
 */
typedef struct banker_st {

	uint ticket_served;
	pthread_mutex_t mu_ticket_served;

	sem_t waiting_room;

	file_t *file;
	customer_t *customer;
	int d1;
	int d0;
	double p;

} banker_t;

uint ticket_tot;
pthread_mutex_t mu_ticket_tot;

bool init_banker(banker_t* b, file_t* f, int d1, double p);

customer_t* add_customer(file_t *file);
void served_first_customer(banker_t *b);
void delete_first_customer(file_t *file);

void* customer_threads(void* b);
void* banker_threads(void* b);

double random_n();
void erreur_input_arg();




