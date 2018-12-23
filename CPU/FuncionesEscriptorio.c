#include "gs.h"
#include"protocolos.h"
#include "FuncionesEscriptorio.h"
extern int fm9;
extern int safa;
extern int diego;
extern t_log* logger;
int soyelpinchecpu = 1;

int abrir(char* path,DTB*dtb) {
	int operacion = 2;
	send(diego, &operacion, 4, 0);
	//ahora hay que mandar el id seguido del path en un paquete
	//mando Header con tamnio
	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+strlen(path)+1;
	enviarHeaderConHandshake(head,diego,logger);
	free(head);
	//armar paquete
	void*paquete=malloc(4+strlen(path)+1);
	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,path,strlen(path)+1);
	//mandar paquete y liberarlo
	send(diego,paquete,4+strlen(path)+1,0);
	free(paquete);
	log_info(logger,"paquete con id y path mandado al diego");
	return 200; //Se manda a bloquear
}

int asignar(char* pathYDatos,DTB*dtb) { //asignar /equipos/barca.txt 9 lauturrop
	int asignar2 = 2;
	int lineaAModificar;
	int memoryP;
	int rtaFM9;
	int tamanioDatos;
	char** lineaEntera=string_n_split(pathYDatos,3, " ");
	char *path = lineaEntera[0];
	char *lineaEnString = lineaEntera[1];
	char *datos = lineaEntera[2];
	lineaAModificar = atoi(lineaEnString);
	memoryP=obtenerMemoryPointer(dtb->tablaDeDirecciones,path);
	if (memoryP == -1) //Si NO esta abierto
	{
		free(path);
		free(lineaEnString);
		free(datos);
		free(lineaEntera);
		free(datos);
		return -1; //Error 40001
	}
	else{
		log_info(logger,"Pasando los datos para la modificacion a FM9");
		void* paquete = malloc(16);//(base,offset,pid,ACCION) asi se debe cargar
		memcpy(paquete,&memoryP,4);//base seg
		memcpy(paquete+4,&lineaAModificar,4);//offset
		memcpy(paquete+4+4,&dtb->ID,4);//PID del proceso
		memcpy(paquete+4+4+4,&asignar2,4);//pedir m0dificacion al fm9
		send(fm9,&soyelpinchecpu,4,0);
		send(fm9,paquete,16,0);
		//printf("ENVIADO ACCION: %d\n",asignar2);
		sleep(1);
		tamanioDatos = strlen(datos)+1;
		send(fm9,&tamanioDatos,4,0);
		send(fm9,datos,tamanioDatos,0);
		//printf("ENVIADO\n");
		free(paquete);
		free(path);
		free(lineaEnString);
		free(datos);
		free(lineaEntera);
		recv(fm9,&rtaFM9,4,MSG_WAITALL);
		if(rtaFM9>0)
			return 100;

		printf("ERROR: 20002 \n");
		return -1;
	}
	//free(path);
	//free(lineaEnString);//creo que estos habia que liberar pero no se si tira seg fault puede ser esto
	//free(datos);
}

int wait(char* parametro,DTB*dtb) {
	// primero, safa puede retener el recurso? safa lo valida
	int wait=3;
	send(safa,&wait,4,0);

	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+strlen(parametro)+1;
	enviarHeaderConHandshake(head,safa,logger);
	free(head);

	void*paquete=malloc(4+strlen(parametro)+1);

	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,parametro,strlen(parametro)+1);

	send(safa,paquete,4+strlen(parametro)+1,0);
	free(paquete);

	int rta=0;
	recv(safa,&rta,4,MSG_WAITALL);

	//si es 1, no es 0
	if (rta==1){
		return 100;
	}else
	{
		return 200;
	}

}

int signal(char* parametro,DTB*dtb) {
	int signal=4;
	send(safa,&signal,4,0);

	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+strlen(parametro)+1;
	enviarHeaderConHandshake(head,safa,logger);
	free(head);

	void*paquete=malloc(4+strlen(parametro)+1);

	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,parametro,strlen(parametro)+1);

	send(safa,paquete,4+strlen(parametro)+1,0);
	free(paquete);

	int rta=0;
	recv(safa,&rta,4,MSG_WAITALL);

	return 100;
}

int closeTP(char* pathACerrar,DTB*dtb) {
	int recerrrar = 3;
	int lineaAModificar=0;
	int memoryP;
	memoryP=obtenerMemoryPointer(dtb->tablaDeDirecciones,pathACerrar);
	if (memoryP == -1) //Si NO esta abierto
	{
		return 10; //Error 40001
	}
	else{
		log_info(logger,"Pasando los datos para liberar la memoria a FM9");
		void* paquete = malloc(16);//(base,offset,pid,ACCION) asi se debe cargar
		memcpy(paquete,&memoryP,4);//base seg
		memcpy(paquete+4,&lineaAModificar,4);//offset
		memcpy(paquete+4+4,&dtb->ID,4);//PID del proceso
		memcpy(paquete+4+4+4,&recerrrar,4);//pedir m0dificacion al fm9
		send(fm9,&soyelpinchecpu,4,0);
		send(fm9,paquete,16,0);
		sleep(0.1);

		for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
			int memoryPointerElemento=0;
			memcpy(&memoryPointerElemento,list_get(dtb->tablaDeDirecciones,i),4);
			if(memoryPointerElemento==memoryP){
				list_remove_and_destroy_element(dtb->tablaDeDirecciones,i,(void*)liberarDTBLista);
			}
		}
		free(paquete);
	}

	return 100;
}
void closeEscriptorio(DTB*dtb){
	int recerrrar = 3;
	int lineaAModificar=0;
	void* paquete = malloc(16);//(base,offset,pid,ACCION) asi se debe cargar
	memcpy(paquete,&dtb->memoryPointerEscriptorio,4);//base seg
	memcpy(paquete+4,&lineaAModificar,4);//offset
	memcpy(paquete+4+4,&dtb->ID,4);//PID del proceso
	memcpy(paquete+4+4+4,&recerrrar,4);//pedir m0dificacion al fm9
	send(fm9,&soyelpinchecpu,4,0);
	send(fm9,paquete,16,0);
	free(paquete);
}

int flush(char* pathAGuardar, DTB*dtb) {

	int operacion = 3;

	int lineaAModificar=0;
	int memoryP;
	memoryP=obtenerMemoryPointer(dtb->tablaDeDirecciones,pathAGuardar);
	if (memoryP == -1)
	{
		return 6; //Error 30001
	}

	send(diego, &operacion, 4, 0);
	//ahora hay que mandar el id seguido del path en un paquete
	//mando Header con tamnio
	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+4+strlen(pathAGuardar)+1;
	enviarHeaderConHandshake(head,diego,logger);
	free(head);
	//armar paquete
	void*paquete=malloc(4+4+strlen(pathAGuardar)+1);
	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,&memoryP,4);
	memcpy(paquete+8,pathAGuardar,strlen(pathAGuardar)+1);
	//mandar paquete y liberarlo
	send(diego,paquete,4+4+strlen(pathAGuardar)+1,0);
	free(paquete);
	log_info(logger,"paquete con id y path mandado al diego");

	return 200;
}

int crear(char* pathYcantidadDeLineas,DTB*dtb) {
	int operacion=4;
	char** lineaEntera=string_split(pathYcantidadDeLineas, " ");
	char* path = lineaEntera[0];
	log_info(logger,"el path es %s",path);
	char* numEnString = lineaEntera[1];
	log_info(logger,"el numero de lineas es %d",numEnString);
	int num = atoi(numEnString);
	send(diego,&operacion,4,0); //Le mando que es "crear"
//	enviarIntconString(path, num, diego, logger);

	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+4+strlen(path)+1;
	enviarHeaderConHandshake(head,diego,logger);
	free(head);
	//armar paquete
	void*paquete=malloc(4+4+strlen(path)+1);
	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,&num,4);
	memcpy(paquete+8,path,strlen(path)+1);
	//mandar paquete y liberarlo
	send(diego,paquete,4+4+strlen(path)+1,0);
	free(paquete);
	log_info(logger,"paquete con id,cantidadLineas y path mandado al diego");

	free(path);
	free(numEnString);
	free(lineaEntera);
	return 200; //BLOQUEA EL PROCESO
}

int borrar(char* path,DTB* dtb) {
	int operacion = 5;
	send(diego, &operacion, 4, 0);

	header*head=malloc(sizeof(header));
	head->id=0;
	head->sizePayload=4+strlen(path)+1;
	enviarHeaderConHandshake(head,diego,logger);
	free(head);

	//armar paquete
	void*paquete=malloc(4+strlen(path)+1);
	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,path,strlen(path)+1);
	//mandar paquete y liberarlo
	send(diego,paquete,4+strlen(path)+1,0);
	free(paquete);
	log_info(logger,"paquete con id y path mandado al diego");

	return 200; //BLOQUEA EL PROCESO

}

int obtenerMemoryPointer(t_list* tabla,char* pathBusc){
	int tamPath=0;
	int mem;
	void* paquete;
	for(int i=0;i<list_size(tabla);i++){
		paquete = list_get(tabla,i);
		memcpy(&tamPath,paquete+4,4);
		char*path=malloc(tamPath);
//		memcpy(path,list_get(tabla,i)+8,tamPath);
		memcpy(path,paquete+8,tamPath);
		if(string_equals_ignore_case(path,pathBusc)){
			free(path);
			memcpy(&mem,paquete,4);
			return mem;
		}
		free(path);
	}
	return -1;
}
/*
 *  int verificarSiEstaEnListaDtb(char*patheando,DTB *dtb){
	for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
	int tamPath=0;
	memcpy(&tamPath,list_get(dtb->tablaDeDirecciones,i)+4,4);
	char*path=malloc(tamPath);
	memcpy(path,list_get(dtb->tablaDeDirecciones,i)+8,tamPath);
	if(string_equals_ignore_case(patheando,path)){
		free(path);
		return 1;
	}
	free(path);
}
	return 0;

}
//si crear es negativo no tiene que mandar cantidadLineas
void enviarADiegoHeaderYPayload(char* path,DTB*dtb){
		header*head=malloc(sizeof(header));
		head->id=0;
		head->sizePayload=4+strlen(path)+1+4;
		enviarHeaderConHandshake(head,diego,logger);
		free(head);
		//armar paquete

		void*paquete=malloc(4+strlen(path)+1);
		memcpy(paquete,&dtb->ID,4);
		memcpy(paquete+4,path,strlen(path)+1);
		send(diego,paquete,4+strlen(path)+1,0);
		free(paquete);
	    log_info(logger,"paquete con id y path mandado al diego");

}
int abrir(char* path,DTB*dtb) {
	int operacion = 2;

	if (verificarSiEstaEnListaDtb(path,dtb)) //Si ya esta abierto
	{
		return 100; //Sigue la ejecucion
	}

	send(diego, &operacion, 4, 0);
	enviarADiegoHeaderYPayload(path,dtb);
	return 200; //Se manda a bloquear
}

int asignar(char* pathYDatos) {
	char *path = (char *) string_split(pathYDatos, " ")[0];
	char *lineaEnString = (char *) string_split(pathYDatos, " ")[1];
	char *datos = (char *) string_split(pathYDatos, " ")[2];
	return 100;
}

int wait(char* parametro) {
	return 100;
}

int signal(char* parametro) {
	return 100;
}

int Close(char* pathACerrar,DTB*dtb) {

	if (verificarSiEstaEnListaDtb(pathACerrar,dtb)) //Si NO esta abierto
	{
		return 10; //Error 40001
	}

	return 100;
}

int flush(char* path,DTB*dtb) {
	if (verificarSiEstaEnListaDtb(path,dtb)) //Si NO esta abierto
	return 6; //Error 30001
	int operacion = 3;
	send(diego, &operacion, 4, 0);
	//ahora hay que mandar el id seguido del path en un paquete
	//mando Header con tamnio
	enviarADiegoHeaderYPayload(path,dtb);
	return 200;
}

int crear(char* pathYcantidadDeLineas,DTB* dtb) {
	int operacion=4;
	send(diego,&operacion,4,0);
	char* path = (char*) string_split(pathYcantidadDeLineas, " ")[0];
	log_info(logger,"el path es %s",path);
	char* numEnString = (char*) string_split(pathYcantidadDeLineas, " ")[1];
	log_info(logger,"el numero de lineas es %d",numEnString);
	int num = atoi(numEnString);
	enviarADiegoHeaderYPayload(path,dtb);
	send(diego,&num,4,0);
	free(path);
	free(numEnString);
	return 200;
}

int borrar(char* path,DTB *dtb) {
	int operacion = 5;
	send(diego,&operacion,4,0);
	enviarADiegoHeaderYPayload(path,dtb);
	return 200;
}**/

