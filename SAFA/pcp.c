#include "planificadores.h"

extern sem_t sem_dummyCargado;
extern int sentenciasNEW;
extern t_list* colaEXEC;
extern t_list*colaREADY;
extern t_list*colaPRIORIDAD;
extern t_list*colaDUMMY;
extern pthread_mutex_t lock;
extern t_config*config;
extern t_log* logger;
extern sem_t sem_planificar;
extern int dummyDisponible;

int volvioUnDTB;
int procesoContado;
time_t tiempoEnvio;

void pcp() {

	volvioUnDTB=1;
	int esReady=0;
	int sePlanifico=0;
	while(1){

		 sem_wait(&sem_planificar);

		DTB*dtbAMandar=malloc(sizeof(DTB));
		sleep(config_get_int_value(config,"RETARDO_PLANIF")/100);

	pthread_mutex_lock(&lock);

		if(dummyDisponible==1 && list_size(colaDUMMY)>0){

		planificarDUMMY(dtbAMandar);
		dummyDisponible--;
		sePlanifico++;
		log_info(logger,"se va a enviar un dummy!");
	}else
		if(list_size(colaREADY)!=0||list_size(colaPRIORIDAD)!=0){
		 planificar(dtbAMandar,config_get_string_value(config,"ALGORITMO"),logger);
		 		dtbAMandar->estado=EXEC;
		 		sePlanifico++;
		 		esReady++;
		 		log_info(logger,"se va a enviar un proceso!");
		}

	pthread_mutex_unlock(&lock);

	if(volvioUnDTB==1){
			procesoContado=dtbAMandar->ID;
			tiempoEnvio=time(NULL);
			volvioUnDTB--;
	}

	if(sePlanifico==1){
		enviarDTBACPU(dtbAMandar,logger);
		if(esReady==1){
		list_add(colaEXEC,dtbAPaquete(dtbAMandar));
		esReady--;
		}

		log_info(logger,"se envio un proceso a un cpu!");
		liberar_lista(dtbAMandar->tablaDeDirecciones);
		list_destroy(dtbAMandar->tablaDeDirecciones);
		free(dtbAMandar->path);
		free(dtbAMandar);
		sePlanifico--;
	}

	if(dummyDisponible==0)
			 sem_wait(&sem_dummyCargado);

		}
}
