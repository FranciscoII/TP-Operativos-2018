#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "gs.h"
#include <semaphore.h>
#include"comunicacionDAM.h"
#include"instrucciones.h"
#include"inicializacionMDJ.h"
#include<commons/config.h>

sem_t sem_plp;
sem_t sem_liberar_dir;
t_log *logger;
t_config* config;
t_config* cmetadata;
int TAMBUFFER;
int CANTIDADBLOQUES;
int  TAM_BLOQUE;
pthread_mutex_t lock;
int DAM;
int serverMDJ;
char* DIRECTORIOACTUAL;
char* dirRaiz;
int main(void) {
	pthread_mutex_init(&lock,NULL);
	inicio();
	pthread_t h_consola;//hilo
	pthread_t h_recepcionesDAM;//hilo
	serverMDJ = crear_server(config_get_string_value(config,"IP"),config_get_string_value(config,"PUERTO"),logger);
	pthread_create(&h_consola, NULL, (void *) consolaMDJ, NULL);
	DAM = esperandoUnaConexion(serverMDJ,logger,1);
	pthread_create(&h_recepcionesDAM, NULL, (void *) recepcionesDAM, NULL);
	pthread_detach(h_recepcionesDAM);
	pthread_join(h_consola, NULL);
	//cambio push forzado!
	log_destroy(logger);
	config_destroy(config);
	return 0;
}

//Esta version tiene esto para ver si esta todo en orden.





