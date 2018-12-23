#include "planificadores.h"

extern sem_t sem_dummyCargado;
extern int gradoMultiprogramacion;
extern pthread_mutex_t lock;
extern t_list* colaAUXILIAR;
extern t_list* colaDUMMY;
extern t_log* logger;
extern sem_t sem_se_cargo_DTB;
extern sem_t sem_planificar;
extern sem_t multiprogramacion;
int dtbMemoryPointer;

void plp_cola() {

	while (1)

	{
		sem_wait(&sem_se_cargo_DTB); //Se espera a que la cola New tenga un dtb al menos

		sem_wait(&multiprogramacion);


	pthread_mutex_lock(&lock);
		gradoMultiprogramacion++;
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTB(dtb,list_get(colaAUXILIAR,0),logger);
		list_add(colaDUMMY,dtbAPaquete(dtb));
		list_remove_and_destroy_element(colaAUXILIAR,0,(void*)liberarDTBLista);
		sem_post(&sem_planificar);
	pthread_mutex_unlock(&lock);
		log_info(logger,"Se mando un dummy a ready!");


		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
	}
}

