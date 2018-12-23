#include "planificadores.h"

extern int socketDiego;
extern int procesosTerminados;
extern t_log* logger;
extern t_config*config;
extern pthread_mutex_t lock;
extern t_list*colaNEW;
extern t_list*colaBLOQ;
extern t_list*colaREADY;
extern t_list*colaEXEC;
extern t_list*colaEXIT;
extern t_list*colaRecursos;
extern t_list*colaPRIORIDAD;
extern clock_t start;
extern int gradoMultiprogramacion;
extern sem_t sem_planificar;
extern t_list*colaDUMMY;
extern sem_t multiprogramacion;
extern t_list*colaTIEMPOS;
extern pthread_mutex_t lockMetricas;


void aniadirAColaDeTiempos(time_t tiempoEnvio,time_t tiempoLlegada){

int tiempo=tiempoLlegada-tiempoEnvio;
int*tiempoTotal=malloc(4);
memcpy(tiempoTotal,&tiempo,4);
pthread_mutex_lock(&lockMetricas);

if(list_size(colaTIEMPOS)==50)
	list_remove_and_destroy_element(colaTIEMPOS,0,(void*)liberarDTBLista);

list_add(colaTIEMPOS,tiempoTotal);
pthread_mutex_unlock(&lockMetricas);
}

void cerrarEnMemoria(DTB*dtb){

dtb->FlagInic=2;
dtb->estado=READY;
enviarDTBACPU(dtb,logger);
dtb->FlagInic=1;
dtb->estado=EXIT;
}
void actualizarNEW(int sentenciasNEW){
	for(int i=0;i<list_size(colaNEW);i++){

		int sentencias=0;
		memcpy(&sentencias,list_get(colaNEW,i)+32,4);
		sentencias++;
		memcpy(list_get(colaNEW,i)+32,&sentencias,4);
	}

}
void planificarDUMMY(DTB*dtb){
	cargarDTB(dtb,list_get(colaDUMMY,0),logger);
	list_remove_and_destroy_element(colaDUMMY,0,(void*)liberarDTBLista);
}

void llenarDTBYPonerEnReady(diego_safa* rutina){
	int posicion=0;
	if(estaElProcesoEnNEW(rutina->id,&posicion,logger)){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTB(dtb,list_get(colaNEW,posicion),logger);
		dtb->IO_Bound=atoi(rutina->path);
		dtb->IO_Bound++;
		dtb->IO_Exec++;
		dtb->memoryPointerEscriptorio=rutina->memoryPointer;
		dtb->FlagInic++;
		dtb->estado= READY;

		list_add(colaREADY,dtbAPaquete(dtb));
		list_remove_and_destroy_element(colaNEW,posicion,(void*)liberarDTBLista);

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
	}
}

void cambiarGradoMP(char*nuevoGrado){

	int sval=0;
	sem_getvalue(&multiprogramacion,&sval);

	if(sval==config_get_int_value(config,"MULTIPROGRAMACION")){
	printSafa("El nuevo grado de multiprogramación sera: ");
		printf("\x1b[33m%s\x1b[0m\n",nuevoGrado);
		config_set_value(config,"MULTIPROGRAMACION",nuevoGrado);
		config_save(config);
		sem_init(&multiprogramacion,0,config_get_int_value(config,"MULTIPROGRAMACION"));
	}
	else{
		printSafa("Espera a que terminen de ejecutar los procesos actuales para cambiar el grado de MP");
	}
}

void cambiarRetardo(char*nuevoRetardo){
	printSafa("El nuevo Retardo sera: ");
		printf("\x1b[33m%s\x1b[0m\n",nuevoRetardo);
		config_set_value(config,"RETARDO_PLANIF",nuevoRetardo);
		config_save(config);
}

void cambiarQuantum(char* nuevoQuantum){
	printSafa("El nuevo Quantum sera: ");
		printf("\x1b[33m%s\x1b[0m\n",nuevoQuantum);
		config_set_value(config,"QUANTUM",nuevoQuantum);
		config_save(config);
}

void cambiarAlgoritmo(char*nuevoAlgoritmo){
	printSafa("El nuevo algoritmo de planificacion será: ");
	printf("\x1b[33m%s\x1b[0m\n",nuevoAlgoritmo);
	config_set_value(config,"ALGORITMO",nuevoAlgoritmo);
	config_save(config);
}

void cargarDTBConMasIOYBorrar(DTB*dtb){
	int i;
	int IOMaximo=0;
	int procesoConMasIOBound=0;
	for(i=0;i<list_size(colaREADY);i++){
		int IOTotal=0;
		memcpy(&IOTotal,list_get(colaREADY,i)+24,4);
		int IOExec=0;
		memcpy(&IOExec,list_get(colaREADY,i)+28,4);

		int IOBound=IOTotal-IOExec;
		if(IOBound>IOMaximo){
			IOMaximo=IOBound;
			procesoConMasIOBound=i;
		}
	}
	cargarDTBSinInformar(dtb,list_get(colaREADY,procesoConMasIOBound),logger);
	list_remove_and_destroy_element(colaREADY,procesoConMasIOBound, (void*) liberarDTBLista);
}
void status(){
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO"),"VRR")){
		printSafa("///////////////////////////////////////////////////////\n");
		printSafa("Grado MP \x1b[31mMaximo\x1b[33m= ");
			printf("%d  ",config_get_int_value(config,"MULTIPROGRAMACION"));
			printSafa("Algoritmo actual= ");
			printf("%s\n",config_get_string_value(config,"ALGORITMO"));
			printSafa("Grado MP actual= ");
			printf("%d ",gradoMultiprogramacion);
			printSafa(" Quantum= ");
			printf("%d\n",config_get_int_value(config,"QUANTUM"));
			printSafa("Procesos finalizados= ");
			printf("%d\n",procesosTerminados);
			printSafa("\n");
			printSafa("\t NEW: ");
			informarCola(colaNEW);
			printSafa("\t READY: ");
			informarCola(colaREADY);
			printSafa("\t *PRIORIDAD: ");
			informarCola(colaPRIORIDAD);
			printSafa("\t BLOQ: ");
			informarCola(colaBLOQ);
			printSafa("\t EXEC: ");
			informarCola(colaEXEC);
			printSafa("\t EXIT: ");
			informarCola(colaEXIT);
			printSafa("///////////////////////////////////////////////////////\n");
	}else{
	printSafa("///////////////////////////////////////////////////////\n");
	printSafa("Grado MP \x1b[31mMaximo\x1b[33m= ");
			printf("%d  ",config_get_int_value(config,"MULTIPROGRAMACION"));
			printSafa("Algoritmo actual= ");
			printf("%s\n",config_get_string_value(config,"ALGORITMO"));
	printSafa("Grado MP actual= ");
	printf("%d ",gradoMultiprogramacion);
	printSafa(" Quantum= ");
	printf("%d\n",config_get_int_value(config,"QUANTUM"));
	printSafa("Procesos Terminados correctamente= ");
	printf("%d\n",procesosTerminados);
	printSafa("\n");
	printSafa("\t NEW: ");
	informarCola(colaNEW);
	printSafa("\t READY: ");
	informarCola(colaREADY);
	printSafa("\t BLOQ: ");
	informarCola(colaBLOQ);
	printSafa("\t EXEC: ");
	informarCola(colaEXEC);
	printSafa("\t EXIT: ");
	informarCola(colaEXIT);
	printSafa("///////////////////////////////////////////////////////\n");
	}
}

void actualizarRecursos(){
	for(int i=0;i<list_size(colaRecursos);i++){
		recurso*resource=list_get(colaRecursos,i);
		for(int k=0;k<list_size(colaEXIT);k++){
			int id=0;
			memcpy(&id,list_get(colaEXIT,k),4);
			if(resource->proceso==id){
				resource->instancias++;
				otorgarRecurso(resource);
			}
		}
	}

}
void moverLaCola(recurso* resource){
int cantElementos=-1*(resource->instancias-1);

if(cantElementos>0){
	resource->proceso=resource->procesosEnCola[0];
}
	for(int i=0;i<cantElementos;i++){
		resource->procesosEnCola[i]=resource->procesosEnCola[i+1];
	}

}

void otorgarRecurso(recurso* resource){
	if(resource->instancias==1){
		resource->proceso=0;
		return;
	}
	int posicion=0;
	diego_safa*inutil=malloc(sizeof(diego_safa));
	 if(estaElProcesoEnBLOQ(resource->procesosEnCola[0],&posicion,logger)){
		desbloquearProceso(resource->procesosEnCola[0],logger,inutil,"recurso",posicion);//primero de la cola
		sem_post(&sem_planificar);
	 }
	 free(inutil);
	moverLaCola(resource);
}

void liberarOCrearRecurso(int proceso,char*nombreRec){

	for(int i=0;i<list_size(colaRecursos);i++){
		recurso*resource=list_get(colaRecursos,i);
		if(string_equals_ignore_case(nombreRec,resource->nombre)){
			resource->instancias++;
			otorgarRecurso(resource);
			return;
		}
	}
	crearRecurso(nombreRec);

}
void operacionSignal(int socket){

	int tamanioRecurso=getHeaderSendHandshake(socket,logger);
	void*paquete=malloc(tamanioRecurso);
	recv(socket,paquete,tamanioRecurso,MSG_WAITALL);
	int proceso=0;
	memcpy(&proceso,paquete,4);
	char*recurso=malloc(tamanioRecurso-4);
	memcpy(recurso,paquete+4,tamanioRecurso-4);

	liberarOCrearRecurso(proceso,recurso);

	int respuestaRecurso=1;
	send(socket,&respuestaRecurso,4,0);
	free(paquete);
	free(recurso);
}

int intentarOtorgar(int proceso,recurso* resource){
	if(resource->instancias<=0){
	resource->procesosEnCola[-1*resource->instancias]=proceso;
	resource->instancias--;
	return 0;
}
	resource->proceso=proceso;
	resource->instancias--;
	return 1;
}
void crearRecurso(char *nombreRec){
	recurso*resource=malloc(sizeof(recurso));
	resource->instancias=1;
	resource->nombre=malloc(strlen(nombreRec)+1);
	list_add(colaRecursos,resource);
}

void crearRecursoYOtorgar(int proceso,char *nombreRec){
	recurso*resource=malloc(sizeof(recurso));
	resource->proceso=proceso;
	resource->instancias=0;
	resource->nombre=malloc(strlen(nombreRec)+1);
	memcpy(resource->nombre,nombreRec,strlen(nombreRec)+1);
	list_add(colaRecursos,resource);
}

int buscarRecursoYOtorgar(int proceso,char* nombreRec){
	if(list_size(colaRecursos)==0){
		//creamos el recurso
		crearRecursoYOtorgar(proceso,nombreRec);
		return 1;
	}
	for(int i=0;i<list_size(colaRecursos);i++){
		recurso*resource=list_get(colaRecursos,i);
		if(string_equals_ignore_case(nombreRec,resource->nombre)){

			return intentarOtorgar(proceso,resource); //0 en caso de no poder, 1 en caso de poder
		}
	}

	crearRecursoYOtorgar(proceso,nombreRec);
	return 1;

}

void operacionWait(int socket){

	int tamanioRecurso=getHeaderSendHandshake(socket,logger);
	void*paquete=malloc(tamanioRecurso);
	recv(socket,paquete,tamanioRecurso,MSG_WAITALL);
	int proceso=0;
	memcpy(&proceso,paquete,4);
	char*recurso=malloc(tamanioRecurso-4);
	memcpy(recurso,paquete+4,tamanioRecurso-4);
	int respuestaRecurso=buscarRecursoYOtorgar(proceso,recurso); //respuesta recurso= 0 esta ocupado, si es 1 se le otorgó
	send(socket,&respuestaRecurso,4,0);
	free(paquete);
	free(recurso);
}

double promedioEXIT(){

	int procesosTerminados=0;
	int pcTotal=0;

				for(int i=0;i<list_size(colaEXIT);i++){
					DTB*dtb=malloc(sizeof(DTB));
					cargarDTBSinInformar(dtb,list_get(colaEXIT,i),logger);
					procesosTerminados++;
					pcTotal+=dtb->PC;
					liberar_lista(dtb->tablaDeDirecciones);
					list_destroy(dtb->tablaDeDirecciones);
					free(dtb->path);
					free(dtb);
				}
if (pcTotal==0){
	return 0;
}


return ((double)pcTotal/(double)procesosTerminados);
}
void mostrarSentenciasNEW(DTB*dtb){
	printSafa("//////////////////////////////////////////////////////////////////\n\t\tMetricas DTB\n\n");
		printSafa("La cantidad de sentencias ejecutadas en NEW del proceso son: ");
		printf("%d\n\n",dtb->sentNEW);
		printSafa("//////////////////////////////////////////////////////////////////\n");


}

void mostrarMetricasDTB(char* idDTB){

	for(int i=0;i<strlen(idDTB);i++){
			if(!isdigit(idDTB[i])){
				printSafa("los procesos se identifican a partir del numero 1000\n por favor ingrese un numero...\n");
				return;
			}
		}
	int id=atoi(idDTB);
	int posicion=0;
//Cant. de sentencias ejecutadas que esperó un DTB en la cola NEW

			if(estaElProcesoEnREADY(id,&posicion,logger)){
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTBSinInformar(dtb,conseguirProcesoEnREADY(id,logger),logger);
				mostrarSentenciasNEW(dtb);
				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);

				return;

			}else if(estaElProcesoEnBLOQ(id,&posicion,logger)){
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTBSinInformar(dtb,conseguirProcesoEnBLOQ(id,logger),logger);
				mostrarSentenciasNEW(dtb);
				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);

				return;

			}else if(estaElProcesoEnEXEC(id,&posicion,logger)){
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTBSinInformar(dtb,conseguirProcesoEnEXEC(id,logger),logger);
				mostrarSentenciasNEW(dtb);
				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);

				return;
			}else if(estaElProcesoEnEXIT(id,&posicion,logger)){
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTBSinInformar(dtb,conseguirProcesoEnEXIT(id,logger),logger);
				mostrarSentenciasNEW(dtb);
				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);

				return;
			}
					//nunca se llego a crear la lista en este caso
				printSafa("No existe un proceso ");
				printf("%d",id);
				printSafa(" en el sistema \n");

}



void mostrarMetricasSistema(){
	//Cant.de sentencias ejecutadas promedio del sistema por proceso que usaron a “El Diego”
	//Porcentaje de las sentencias ejecutadas promedio que fueron a “El Diego”
	//Cant. de sentencias ejecutadas prom. del sistema para que un DTB termine en la cola EXIT
	//Tiempo de Respuesta promedio del Sistema
	int cantTotalEjecutadas=0;
	int cantTotalBloqueantes=0;
	float cantProcesos=0;

	for(int i=0;i<list_size(colaREADY);i++){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaREADY,i),logger);

		cantProcesos++;
		cantTotalEjecutadas+=dtb->PC;
		cantTotalBloqueantes+=dtb->IO_Exec;

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
	}
	for(int i=0;i<list_size(colaBLOQ);i++){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaBLOQ,i),logger);

			cantProcesos++;
			cantTotalEjecutadas+=dtb->PC;
			cantTotalBloqueantes+=dtb->IO_Exec;

			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		}
	for(int i=0;i<list_size(colaEXEC);i++){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaEXEC,i),logger);

			cantProcesos++;
			cantTotalEjecutadas+=dtb->PC;
			cantTotalBloqueantes+=dtb->IO_Exec;

			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		}
	for(int i=0;i<list_size(colaEXIT);i++){
				DTB*dtb=malloc(sizeof(DTB));
				cargarDTBSinInformar(dtb,list_get(colaEXIT,i),logger);

				cantProcesos++;
				cantTotalEjecutadas+=dtb->PC;
				cantTotalBloqueantes+=dtb->IO_Exec;

				liberar_lista(dtb->tablaDeDirecciones);
				list_destroy(dtb->tablaDeDirecciones);
				free(dtb->path);
				free(dtb);
			}

	double cantEjecutadasPromedio;
if(cantProcesos==0)
	cantEjecutadasPromedio=0;
else{
	cantEjecutadasPromedio=(double)cantTotalBloqueantes/(double)cantProcesos;}

	int porcentageSentenciasAlDAM;
if(cantTotalBloqueantes==0)
	porcentageSentenciasAlDAM=0;
else{
	porcentageSentenciasAlDAM=((double)cantTotalBloqueantes/(double)cantTotalEjecutadas)*100;}

pthread_mutex_lock(&lockMetricas);
	double tiempoRespuesta=0;
		for(int i=0;i<list_size(colaTIEMPOS);i++){
			int tiempo;
			memcpy(&tiempo,list_get(colaTIEMPOS,i),4);
			tiempoRespuesta+=tiempo;
	}
		if(list_size(colaTIEMPOS)==0){
			tiempoRespuesta=0;
		}else{
			tiempoRespuesta=tiempoRespuesta/list_size(colaTIEMPOS);
		}
pthread_mutex_unlock(&lockMetricas);

	mostrarDatosSistema(cantEjecutadasPromedio,porcentageSentenciasAlDAM,tiempoRespuesta,promedioEXIT());
}
void mostrarDatosSistema(float ejecPromDAM,int porcentageDam,double tiempoRespuesta,float promEXIT){
	printSafa("////////////////////////////////////////////////////////\n\t\tMetricas\n\n");
	printSafa("Promedio de lineas bloqueantes ejecutadas por proceso: ");
	printf("%.2f sentencias \n\n",ejecPromDAM);
	printSafa("Porcentage de lineas bloqueantes de todo el sistema: ");
	printf("%d%% \n\n",porcentageDam);
	printSafa("Promedio se sentencias ejecutadas que llevan a exit a un proceso: ");
	printf("%.2f sentencias\n\n",promEXIT);
	printSafa("Tiempo promedio de uso de CPU: ");
	printf("%.4f segundos    [ultimas 50 ejecuciones]\n\n",tiempoRespuesta);
	printSafa("////////////////////////////////////////////////////////\n");
}





void finalizar(char* idDTB,t_log*logger){

	for(int i=0;i<strlen(idDTB);i++){
			if(!isdigit(idDTB[i])){
				printSafa("los procesos se identifican a partir del numero 1000\n por favor ingrese un numero...\n");
				return;
			}
		}
	int id=atoi(idDTB);
	int posicion=0;

	if(estaElProcesoEnNEW(id,&posicion,logger)){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaNEW,posicion),logger);
			cerrarEnMemoria(dtb);
			list_add(colaEXIT,dtbAPaquete(dtb));
			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		pthread_mutex_lock(&lock);
			procesosTerminados++;
			gradoMultiprogramacion--;
			sem_post(&multiprogramacion);
			destruirProcesoEnNEW(id,logger);
			printSafa("el proceso: ");
			printf("%d",id);
			printSafa(" se ah eliminado del sistema \n");
		pthread_mutex_unlock(&lock);
			return;

		}else if(estaElProcesoEnREADY(id,&posicion,logger)){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaREADY,posicion),logger);
			cerrarEnMemoria(dtb);
			list_add(colaEXIT,dtbAPaquete(dtb));
			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		pthread_mutex_lock(&lock);
			gradoMultiprogramacion--;
			sem_post(&multiprogramacion);
			procesosTerminados++;
			destruirProcesoEnREADY(id,logger);
			printSafa("el proceso: ");
			printf("%d",id);
			printSafa(" se ah eliminado del sistema \n");
		pthread_mutex_unlock(&lock);
			return;

		}else if(estaElProcesoEnBLOQ(id,&posicion,logger)){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaBLOQ,posicion),logger);
			cerrarEnMemoria(dtb);
			list_add(colaEXIT,dtbAPaquete(dtb));
			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		pthread_mutex_lock(&lock);
			procesosTerminados++;
			gradoMultiprogramacion--;
			sem_post(&multiprogramacion);
			destruirProcesoEnBLOQ(id,logger);
			printSafa("el proceso: ");
			printf("%d",id);
			printSafa(" se ah eliminado del sistema \n");
		pthread_mutex_unlock(&lock);
			return;

		}else if(estaElProcesoEnEXEC(id,&posicion,logger)){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaEXEC,posicion),logger);
			cerrarEnMemoria(dtb);
			list_add(colaEXIT,dtbAPaquete(dtb));
			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		pthread_mutex_lock(&lock);
			procesosTerminados++;
			gradoMultiprogramacion--;
			sem_post(&multiprogramacion);
			destruirProcesoEnEXEC(id,logger);
			printSafa("el proceso: ");
			printf("%d",id);
			printSafa(" se ah eliminado del sistema \n");
		pthread_mutex_unlock(&lock);
			return;

		}else if(estaElProcesoEnPRIORIDAD(id,&posicion,logger)){
			DTB*dtb=malloc(sizeof(DTB));
			cargarDTBSinInformar(dtb,list_get(colaPRIORIDAD,posicion),logger);
			cerrarEnMemoria(dtb);
			list_add(colaEXIT,dtbAPaquete(dtb));
			liberar_lista(dtb->tablaDeDirecciones);
			list_destroy(dtb->tablaDeDirecciones);
			free(dtb->path);
			free(dtb);
		pthread_mutex_lock(&lock);
			procesosTerminados++;
			gradoMultiprogramacion--;
			sem_post(&multiprogramacion);
			destruirProcesoEnPRIORIDAD(id,logger);
			printSafa("el proceso: ");
			printf("%d",id);
			printSafa(" se ah eliminado del sistema \n");
		pthread_mutex_unlock(&lock);
			return;
		}
				//nunca se llego a crear la lista en este caso
			printSafa("No existe un proceso ");
			printf("%d",id);
			printSafa(" en el sistema \n");

}

void mostrarDTB(DTB*dtb){
	printSafa("///////////////////////////////////////////////////////\n");
	printf("\x1b[33mid:%d\n\x1b[0m",dtb->ID);
	printf("\x1b[33mflag:%d\n\x1b[0m",dtb->FlagInic);
	printf("\x1b[33mestado:%d\n\x1b[0m",dtb->estado);
	printf("\x1b[33mPC:%d\n\x1b[0m",dtb->PC);
	printf("\x1b[33mQuantum:%d\n\x1b[0m",dtb->quantum);
	printf("\x1b[33mSentencias en NEW:%d\n\x1b[0m",dtb->sentNEW);
	printf("\x1b[33mPuntero a memoria:%d\n\x1b[0m",dtb->memoryPointerEscriptorio);
	printf("\x1b[33mOperaciones I/O totales:%d\n\x1b[0m",dtb->IO_Bound);
	printf("\x1b[33mOperaciones I/O ejecutadas:%d\n\x1b[0m",dtb->IO_Exec);
	printf("\x1b[33mPath:%s\n\x1b[0m",dtb->path);
	printf("\x1b[33mtabla de direcciones:\n\x1b[0m");

	for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
		int memoria=0;
		memcpy(&memoria,list_get(dtb->tablaDeDirecciones,i),4);
		int tamPath=0;
		memcpy(&tamPath,list_get(dtb->tablaDeDirecciones,i)+4,4);
		char*path=malloc(tamPath);
		memcpy(path,list_get(dtb->tablaDeDirecciones,i)+8,tamPath);
		printf("\t\x1b[33mposicion de la lista %d\x1b[0m\n",i);
		printf("\t\x1b[33mespacio en memoria:%d\x1b[0m\n",memoria);
		printf("\t\x1b[33mpath:%s\n\x1b[0m",path);
		printf("\x1b[0m--------------------------------------\n");
		free(path);
	}
	printSafa("///////////////////////////////////////////////////////\n");
}




void mostrarDatosDTB(char* idDTB,t_log*logger){
	for(int i=0;i<strlen(idDTB);i++){
		if(!isdigit(idDTB[i])){
			printSafa("los procesos se identifican a partir del numero 1000\n por favor ingrese un numero...\n");
			return;
		}
	}


	int id=atoi(idDTB);
	int posicion=0;


	if(estaElProcesoEnNEW(id,&posicion,logger)){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaNEW,posicion),logger);
		mostrarDTB(dtb);

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}else if(estaElProcesoEnREADY(id,&posicion,logger)){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaREADY,posicion),logger);
		mostrarDTB(dtb);
		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}else if(estaElProcesoEnBLOQ(id,&posicion,logger)){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaBLOQ,posicion),logger);
		mostrarDTB(dtb);
		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}else if(estaElProcesoEnEXEC(id,&posicion,logger)){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaEXEC,posicion),logger);
		mostrarDTB(dtb);
		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}
			//nunca se llego a crear la lista en este caso
		printSafa("No existe un proceso ");
		printf("%d",id);
		printSafa(" en el sistema \n");

}



void informarCola(t_list* cola){
	printSafa("");
	printf("%d\n",list_size(cola));
	for(int i=0;i<list_size(cola);i++){
		int idproceso=0;
		memcpy(&idproceso,list_get(cola,i),4);
		printSafa("Proceso ");
		printf("%d ",idproceso);
		printSafa("PC-->");
		int programCounter=0;
		memcpy(&programCounter,list_get(cola,i)+12,4);
		printf("%d ",programCounter);

		if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO"),"IOBF")){
		printSafa("IO Bound-->");
		int IOTotal=0;
		memcpy(&IOTotal,list_get(cola,i)+24,4);
		int IOExec=0;
		memcpy(&IOExec,list_get(cola,i)+28,4);
		int IOBound=IOTotal-IOExec;
		printf("%d ",IOBound);
		}

		int esdummy=0;
		memcpy(&esdummy,list_get(cola,i)+4,4);
		if(esdummy==0){
				printf(" (sin cargar en memoria)  ");
			}
		if(esdummy==2){
						printf(" (a liberar de la memoria)  ");
					}
		printf("\n");
	}
	printf("\n");
}
void printSafa(char*palabras){
	printf("\x1b[33m%s\x1b[0m",palabras);
}
int estaElProcesoEnEXEC(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaEXEC); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaEXEC, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}
	return 0;
}
void destruirProcesoEnEXEC(int id,t_log*logger){
	int posicion=0;
	if(estaElProcesoEnEXEC(id,&posicion,logger)){
		list_remove_and_destroy_element(colaEXEC,posicion,liberarDTBLista);
		log_info(logger,"el proceso %d ya no esta en ejecucion",id);
	}
}
void destruirProcesoEnREADY(int id,t_log*logger){
	int posicion=0;
	if(estaElProcesoEnREADY(id,&posicion,logger)){
		list_remove_and_destroy_element(colaREADY,posicion,liberarDTBLista);
		log_info(logger,"el proceso %d ya no esta en ejecucion",id);
	}
}
int estaElProcesoEnREADY(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaREADY); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaREADY, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}
	return 0;
}
void destruirProcesoEnNEW(int id,t_log*logger){
	int posicion=0;
	if(estaElProcesoEnNEW(id,&posicion,logger)){
		list_remove_and_destroy_element(colaNEW,posicion,liberarDTBLista);
		log_info(logger,"el proceso %d ya no esta en ejecucion",id);
	}
}
void destruirProcesoEnPRIORIDAD(int id,t_log*logger){
	int posicion=0;
	if(estaElProcesoEnPRIORIDAD(id,&posicion,logger)){
		list_remove_and_destroy_element(colaPRIORIDAD,posicion,liberarDTBLista);
	}
}
int estaElProcesoEnPRIORIDAD(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaPRIORIDAD); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaPRIORIDAD, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}

	return 0;
}
int estaElProcesoEnNEW(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaNEW); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaNEW, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}

	return 0;
}
void destruirProcesoEnBLOQ(int id,t_log*logger){
	int posicion=0;
	if(estaElProcesoEnBLOQ(id,&posicion,logger)){
		list_remove_and_destroy_element(colaBLOQ,posicion,liberarDTBLista);
		log_info(logger,"el proceso %d ya no esta en ejecucion",id);
	}
}
int estaElProcesoEnBLOQ(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaBLOQ); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaBLOQ, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}

	return 0;
}
int estaElProcesoEnEXIT(int id, int*pos,t_log*logger) {
	for (int i = 0; i < list_size(colaEXIT); i++) {
		int proceso = 0;
		memcpy(&proceso, list_get(colaEXIT, i), 4);
		if (proceso == id) {
			*pos=i;
			return 1;
		}
	}

	return 0;
}
void*conseguirProcesoEnPRIORIDAD(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnPRIORIDAD(proceso,&posicion,logger);
return list_get(colaPRIORIDAD,posicion);
}
void*conseguirProcesoEnEXIT(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnEXIT(proceso,&posicion,logger);
return list_get(colaEXIT,posicion);
}
void*conseguirProcesoEnNEW(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnNEW(proceso,&posicion,logger);
return list_get(colaNEW,posicion);
}
void*conseguirProcesoEnREADY(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnREADY(proceso,&posicion,logger);
return list_get(colaREADY,posicion);
}

void*conseguirProcesoEnEXEC(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnEXEC(proceso,&posicion,logger);
return list_get(colaEXEC,posicion);
}

void*conseguirProcesoEnBLOQ(int proceso,t_log*logger){
int posicion=0;
estaElProcesoEnBLOQ(proceso,&posicion,logger);
return list_get(colaBLOQ,posicion);
}

void desbloquearProceso(int id,t_log*logger,diego_safa*rutina,char*operacion,int posicion) {

	if(string_equals_ignore_case(operacion,"recurso")){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,conseguirProcesoEnBLOQ(id,logger),logger);

		dtb->estado=READY;
		moverSegunAlgoritmo(config_get_string_value(config,"ALGORITMO"),dtb,posicion);

		log_info(logger,"se ha desbloqueado el proceso %d",rutina->id);

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}
	if(string_equals_ignore_case(operacion,"intacto")){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,conseguirProcesoEnBLOQ(id,logger),logger);

		dtb->estado=READY;
		dtb->IO_Exec++;
		moverSegunAlgoritmo(config_get_string_value(config,"ALGORITMO"),dtb,posicion);


		log_info(logger,"se ha desbloqueado el proceso %d",rutina->id);

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;

	}
	if(string_equals_ignore_case(operacion,"abrir")){

	DTB*dtb=malloc(sizeof(DTB));
	cargarDTBSinInformar(dtb,conseguirProcesoEnBLOQ(id,logger),logger);
	//cargado de archivo

	void*paqueteLista=malloc(8+strlen(rutina->path)+1);
	memcpy(paqueteLista,&rutina->memoryPointer,4);
	int tamPath=strlen(rutina->path)+1;
	memcpy(paqueteLista+4,&tamPath,4);
	memcpy(paqueteLista+8,rutina->path,tamPath);
	list_add(dtb->tablaDeDirecciones,paqueteLista);

	//fin de cargado de archivo
	dtb->estado=READY;
	dtb->IO_Exec++;
	moverSegunAlgoritmo(config_get_string_value(config,"ALGORITMO"),dtb,posicion);

	log_info(logger,"se ha desbloqueado el proceso %d",rutina->id);

	liberar_lista(dtb->tablaDeDirecciones);
	list_destroy(dtb->tablaDeDirecciones);
	free(dtb->path);
	free(dtb);
	return;
	}
	if(string_equals_ignore_case(operacion,"flush")){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,conseguirProcesoEnBLOQ(id,logger),logger);
		//cargado de archivo

		liberar_lista(dtb->tablaDeDirecciones);

		//fin de cargado de archivo
		dtb->estado=READY;
		dtb->IO_Exec++;
		moverSegunAlgoritmo(config_get_string_value(config,"ALGORITMO"),dtb,posicion);

		log_info(logger,"se ha desbloqueado el proceso %d",rutina->id);

		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		return;
	}
	if(string_equals_ignore_case(operacion,"tirar")){
		DTB*dtb=malloc(sizeof(DTB));
		cargarDTBSinInformar(dtb,list_get(colaBLOQ,posicion),logger);
		list_add(colaEXIT,dtbAPaquete(dtb));
		liberar_lista(dtb->tablaDeDirecciones);
		list_destroy(dtb->tablaDeDirecciones);
		free(dtb->path);
		free(dtb);
		list_remove_and_destroy_element(colaBLOQ, posicion,
				(void*) liberarDTBLista);
	return;
	}
}

void moverSegunAlgoritmo(char*algoritmo,DTB*dtb,int posicion){
	if(string_equals_ignore_case(algoritmo, "RR") == 1||string_equals_ignore_case(algoritmo, "IOBF") == 1){
		list_add(colaREADY,dtbAPaquete(dtb));
			list_remove_and_destroy_element(colaBLOQ, posicion,
				(void*) liberarDTBLista);
	return;
	}
	if(string_equals_ignore_case(algoritmo, "VRR") == 1){
		if(dtb->quantum<config_get_int_value(config,"QUANTUM")){
			list_add(colaPRIORIDAD,dtbAPaquete(dtb));
			list_remove_and_destroy_element(colaBLOQ, posicion,(void*) liberarDTBLista);
		}else{

			list_add(colaREADY,dtbAPaquete(dtb));
				list_remove_and_destroy_element(colaBLOQ, posicion,
					(void*) liberarDTBLista);
		}
			return;
		}
}

void planificar(DTB*dtb, char* algoritmo, t_log*logger) {
		log_info(logger, "el algoritmo es %s", algoritmo);
	if (string_equals_ignore_case(algoritmo, "RR") == 1) {
		cargarDTBSinInformar(dtb, list_get(colaREADY, 0), logger);
		list_remove_and_destroy_element(colaREADY, 0, (void*) liberarDTBLista);
		return;

	}
	if (string_equals_ignore_case(algoritmo, "VRR") == 1) {
		if(list_size(colaPRIORIDAD)>0){
			cargarDTBSinInformar(dtb, list_get(colaPRIORIDAD, 0), logger);
			list_remove_and_destroy_element(colaPRIORIDAD, 0, (void*) liberarDTBLista);

		}else{
			cargarDTBSinInformar(dtb, list_get(colaREADY, 0), logger);
			list_remove_and_destroy_element(colaREADY, 0, (void*) liberarDTBLista);
		}
		return;
	}
	if (string_equals_ignore_case(algoritmo, "IOBF") == 1) {
			cargarDTBConMasIOYBorrar(dtb);
			return;
		}
	else {
		log_error(logger, "no se llego a planificar ningun algoritmo");
	}
}

