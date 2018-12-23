#include "planificadores.h"

extern CPU procesadores[MAX_CPU];
extern sem_t sem_disponibilidad;
int cpuADisponibilizarPorDiego;



void enviarDTBACPU(DTB*dtbAMandar,t_log*logger){
// semaforo que sume cad vez que hay algo disponible
if(dtbAMandar->estado==EXIT){
	log_info(logger,"el dtb se ha mandado a exit");
	return;
}
	sem_wait(&sem_disponibilidad);
	int i;
	for(i=0;(procesadores[i].disponibilidad)==0;i++); // que me busque uno disponible
	procesadores[i].disponibilidad=0;
	enviarDTB(dtbAMandar,procesadores[i].conexion,logger);
}


