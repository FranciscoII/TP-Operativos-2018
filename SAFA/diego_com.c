#include "planificadores.h"


int ioBound;
extern sem_t sem_dummyCargado;
extern pthread_mutex_t lock;
extern t_config*config;
extern sem_t sem_planificar;
extern sem_t el_diego_ya_hizo_lo_suyo;
extern sem_t diego_me_hablo;
extern sem_t sem_path_en_memoria;
extern sem_t multiprogramacion;
extern t_log* logger;
extern int salida;// agregar salida para que no rompa el exit
extern int bocharDummy;
extern int socketDiego;
extern int dtbMemoryPointer;
extern int gradoMultiprogramacion;
extern int dummyDisponible;
extern t_list*colaEXIT;

void diego_com(){

while(1){

	sem_wait(&diego_me_hablo);
	diego_safa*rutina=malloc(sizeof(diego_safa));
	recibirRutinaDelDiego(rutina,socketDiego,logger);// esto es bloqueante
	log_info(logger,"\n\x1b[33m---Rutina recibida del DAM---\nTipo: %d\nId del proceso: %d\nPuntero de memoria: %d\nPath asociado: %s\n\x1b[0m",rutina->tipo,rutina->id,rutina->memoryPointer,rutina->path);
	sem_post(&el_diego_ya_hizo_lo_suyo);
	switch(rutina->tipo){
			case 1: //el dummy se cargo
				log_info(logger,"llego el dummy ok");
			pthread_mutex_lock(&lock);
				llenarDTBYPonerEnReady(rutina);
				dummyDisponible++;
				sem_post(&sem_dummyCargado);
				sem_post(&sem_planificar);
			pthread_mutex_unlock(&lock);
			log_info(logger,"La operacion dummy fue exitosa!");
				break;

			case 2: //termino un abrir
				;
				int posicion = 0;
				 pthread_mutex_lock(&lock);
				 if(estaElProcesoEnBLOQ(rutina->id,&posicion,logger)){
				desbloquearProceso(rutina->id,logger,rutina,"abrir",posicion);
				sem_post(&sem_planificar);
				 }
				 pthread_mutex_unlock(&lock);
				 log_info(logger,"La operacion abrir fue exitosa!");

				break;

			case 3: //el dummy no se pudo cargar / se bocha el dummy y el dtb original
				log_info(logger,"adios al dummy");
				int posicion5=0;
			pthread_mutex_lock(&lock);
				if(estaElProcesoEnNEW(rutina->id,&posicion5,logger)){
					DTB*dtb=malloc(sizeof(DTB));
					cargarDTB(dtb,conseguirProcesoEnNEW(rutina->id,logger),logger);
					list_add(colaEXIT,dtbAPaquete(dtb));
					destruirProcesoEnNEW(rutina->id,logger);
					gradoMultiprogramacion--;
					liberar_lista(dtb->tablaDeDirecciones);
					list_destroy(dtb->tablaDeDirecciones);
					free(dtb->path);
					free(dtb);
					sem_post(&multiprogramacion);
				}
				dummyDisponible++;
				sem_post(&sem_dummyCargado);
			pthread_mutex_unlock(&lock);
			log_info(logger,"Se ah tenido que tirar el proceso por la operacion dummy fallida!");
				break;

			case 4: //un archivo del proceso no se pudo cargar/ adios proces
				;
				int posicion1 = 0;
				 //espera activa

				if(estaElProcesoEnBLOQ(rutina->id,&posicion1,logger)){
			pthread_mutex_lock(&lock);
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTB(dtb,conseguirProcesoEnBLOQ(rutina->id,logger),logger);
				desbloquearProceso(rutina->id,logger,rutina,"tirar",posicion1);
				gradoMultiprogramacion--;
				sem_post(&multiprogramacion);
			pthread_mutex_unlock(&lock);
				cerrarEnMemoria(dtb);
				log_info(logger,"Se ah tirado el proceso por algun fallo!");
				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);
				}
				log_info(logger,"adios al dtb");
				break;

			case 6://caso No cambiante (crear y borrar)
				;
				int posix;
				pthread_mutex_lock(&lock);
				if(estaElProcesoEnBLOQ(rutina->id,&posix,logger)){
					desbloquearProceso(rutina->id,logger,rutina,"intacto",posix);
					sem_post(&sem_planificar);}
				pthread_mutex_unlock(&lock);
				log_info(logger,"se ah terminado una operacion iobound con exito!");

				break;
			default:
				log_info(logger,"tipo desconocido,chequear envio de paquetes");
		}
		free(rutina->path);
		free(rutina);
	}
}

