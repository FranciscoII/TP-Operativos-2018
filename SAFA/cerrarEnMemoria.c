#include "planificadores.h"


extern sem_t sem_cerrar_memoria;
extern t_list*colaLIBERAR;
extern t_log* logger;

void liberarProcesos(){
	while(1){
		sem_wait(&sem_cerrar_memoria);
		log_info(logger,"Se va a liberar el siguiente dtb");
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTB(dtb,list_get(colaLIBERAR,0),logger);
		cerrarEnMemoria(dtb);
		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		list_remove_and_destroy_element(colaLIBERAR,0,(void*)liberarDTBLista);
	}
}

