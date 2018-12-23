#include "gs.h"
#include <pthread.h>
#include <sys/select.h>
#include <semaphore.h>
#include "planificadores.h"
#define MAX_CPU 10
//variables globales
pthread_mutex_t lock;
pthread_mutex_t lockMetricas;
sem_t sem_dummyCargado;
sem_t sem_plp;
sem_t sem_liberar_dir;
sem_t sem_launch_dummy;
sem_t sem_conexion_inicial;
sem_t sem_plp_cola;
sem_t sem_se_cargo_DTB;
sem_t sem_path_en_memoria;
sem_t sem_planificar;
sem_t sem_disponibilidad;
sem_t sem_diego_y_cpu_conectados;
sem_t diego_me_hablo;
sem_t el_diego_ya_hizo_lo_suyo;
sem_t multiprogramacion;
sem_t sem_cerrar_memoria;


t_config*config;
t_log* logger;
extern t_list* colaNEW;
t_list* colaRecursos;
t_list* colaEXEC;
t_list* colaREADY;
t_list*colaBLOQ;
t_list*colaPRIORIDAD;
t_list*colaAUXILIAR;
t_list*colaDUMMY;
t_list*colaTIEMPOS;
t_list*colaLIBERAR;

char* dirEscript;
int validacionSAFA;
int conexionDAM;
CPU procesadores[MAX_CPU]; //inicialicemos 10 procesadores por ahora
int procesosTerminados;
int bocharDummy;
int gradoMultiprogramacion;
int dummyDisponible;
clock_t start;

//ip "192.168.1.2"
//puerto 8000
//pero usamos loop

int main(int argc, char *argv[]) {

	//printf("\n\x1b[33m \n\x1b[0m"); color safa
	dummyDisponible=1;
	start=clock();
	procesosTerminados=0;
	gradoMultiprogramacion=0;
	bocharDummy=0;
	pthread_mutex_init(&lock,NULL);
	pthread_mutex_init(&lockMetricas,NULL);
	config=config_create("/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/SAFA.txt");

	logger = log_create("SAFA.log", "safa", 0, 0);

	sem_init(&sem_diego_y_cpu_conectados,0,0);

	pthread_t h_conexiones;//majestuoso hilo 6

	pthread_create(&h_conexiones,NULL,(void*)conexiones,NULL);

	pthread_detach(h_conexiones);

	sem_wait(&sem_diego_y_cpu_conectados);//cuando se conecten el diego y el cpu

	colaLIBERAR=list_create();
	colaTIEMPOS=list_create();
	colaPRIORIDAD=list_create();
	colaRecursos=list_create();
	colaREADY = list_create();
	colaBLOQ = list_create();
	colaEXEC=list_create();
	colaAUXILIAR=list_create();
	colaDUMMY=list_create();
	/*validacionSAFA representa que conexion es diego,
	 entonces tambien representa si se realizo la conexion inicial correctamente*/

	if (validacionSAFA > 0) {
		log_info(logger, "SAFA: operativo");

		sem_init(&sem_cerrar_memoria,0,0);
		sem_init(&sem_dummyCargado,0,0);
		sem_init(&multiprogramacion,0,config_get_int_value(config,"MULTIPROGRAMACION"));
		sem_init(&sem_plp, 0, 0);//s1
		sem_init(&sem_liberar_dir, 0, 0);//semaforo para liberar memoria correctamente
		sem_init(&sem_se_cargo_DTB, 0, 0);//s2
		sem_init(&sem_path_en_memoria,0,0);//s3
		sem_init(&sem_planificar,0,0);//s4
		sem_init(&sem_disponibilidad,0,1);//s5 empieza en uno por la conexion incial

		pthread_t h_plp;//hilo2
		pthread_t h_plp_cola;//hilo3
		pthread_t h_diego_com;//hilo4
		pthread_t h_pcp;//hilo5
		pthread_t h_consola;//hilo1
		pthread_t h_cerrarmemoria;//hilo7

		pthread_create(&h_cerrarmemoria, NULL, (void *) liberarProcesos, NULL);
		pthread_create(&h_plp, NULL, (void *) plp, NULL);
		pthread_create(&h_plp_cola, NULL, (void *) plp_cola, NULL);
		pthread_create(&h_diego_com, NULL, (void *) diego_com, NULL);
		pthread_create(&h_pcp, NULL, (void *) pcp, NULL);
		pthread_create(&h_consola, NULL, (void *) consola, NULL);

		pthread_detach(h_cerrarmemoria);
		pthread_detach(h_plp);
		pthread_detach(h_plp_cola);
		pthread_detach(h_diego_com);
		pthread_detach(h_pcp);
		pthread_join(h_consola, NULL);

	} else {
		log_error(logger, "No se pudo salir del estado corrupto");
	}
	pthread_mutex_lock(&lock);
	liberar_lista(colaNEW);
	liberar_lista(colaREADY);
	liberar_lista(colaBLOQ);
	liberar_lista(colaEXEC);
	list_destroy(colaNEW);
	list_destroy(colaEXEC);
	list_destroy(colaREADY);
	list_destroy(colaBLOQ);
	pthread_mutex_unlock(&lock);
	pthread_mutex_destroy(&lock);
	config_destroy(config);
	return 1;
}
