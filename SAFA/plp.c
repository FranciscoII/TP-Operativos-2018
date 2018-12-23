#include "planificadores.h"
extern t_log*logger;
extern sem_t sem_plp;
extern char* dirEscript;
extern sem_t sem_liberar_dir;
extern sem_t sem_se_cargo_DTB;
extern int salida;
extern t_config*config;
extern pthread_mutex_t lock;
//Variables con la cola

extern t_list* colaAUXILIAR;
t_list* colaNEW;

void plp() {

	int codigo = 1000;
	colaNEW = list_create();// se debe crear la cola

	while (1) {

//todo checar que onda el grado de multiprogramacion

			sem_wait(&sem_plp); //Se espera a que consola.c ejecute un nuevo archivo

	if (salida != 1) {// a mas tardar incorporar esto en cada hilo

			DTB*nuevoDTB = malloc(sizeof(DTB));

			//Se crea el nuevo DTB
			nuevoDTB->estado = NEW;
			nuevoDTB->ID = codigo;
			nuevoDTB->FlagInic = 0;
			nuevoDTB->PC = 0;
			nuevoDTB->quantum=config_get_int_value(config,"QUANTUM");
			nuevoDTB->memoryPointerEscriptorio=0;
			nuevoDTB->IO_Bound=0;
			nuevoDTB->IO_Exec=0;
			nuevoDTB->sentNEW=0;
			nuevoDTB->path = malloc(strlen(dirEscript) + 1);
			memcpy(nuevoDTB->path,dirEscript,strlen(dirEscript)+1);
			nuevoDTB->tablaDeDirecciones=list_create();
			free(dirEscript);
		pthread_mutex_lock(&lock);
			list_add(colaNEW,dtbAPaquete(nuevoDTB));
			list_add(colaAUXILIAR,dtbAPaquete(nuevoDTB));
		pthread_mutex_unlock(&lock);

			liberar_lista(nuevoDTB->tablaDeDirecciones);
			list_destroy(nuevoDTB->tablaDeDirecciones);
			free(nuevoDTB->path);
			free(nuevoDTB);

			codigo++;

			log_info(logger,"Se generó un proceso en la cola NEW!");

			sem_post(&sem_se_cargo_DTB); //Se avisa a plp_cola.c que se cargo un DTB

			sem_post(&sem_liberar_dir);// semaforo para que siga la consola

		} else {
			list_destroy(colaNEW);
			break;
		}
	}
}

