#include"comunicacionMDJ.h"
#include "operacionesDAM.h"
extern t_log* logger;
extern t_config* config;
extern int safa;
extern int fm9;
extern int mdj_a_fm9;
//PUNTO_MONTAJE=/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz
char* h = "abrir h.txt\n close h.txt\n xd\n";
//char* p = "abrir h.txt\n close h.txt\n xd\n";
char* p = "crear /equipos/Rafaela 5";
instruccionaMDJ* create_instruction(char* path){
	instruccionaMDJ * instruccion=malloc(sizeof(instruccionaMDJ));
	instruccion->tamanioPath=strlen(path)+1;
	instruccion->path=path;
	return instruccion;
}

 instruccionaMDJ* create_instruction_gd(char *path,off_t offset,size_t size,void* buffer){
 	instruccionaMDJ *instruccion=create_instruction(path);
 	instruccion->cod=GUARDARDATOS;
 	instruccion->offset=offset;
 	instruccion->tam=size;
 	instruccion->buf=malloc(instruccion->tam);
 	memcpy(instruccion->buf,buffer,size);
 	instruccion->nbytes=0;
 	return instruccion;
 }
 instruccionaMDJ* create_instruction_va(char* path){
 	instruccionaMDJ *instruccion=create_instruction(path);
 	instruccion->cod=VALIDARARCHIVO;
 	return instruccion;
 }

 instruccionaMDJ* create_instruction_od(char *path,off_t offset,size_t size){
 	instruccionaMDJ *instruccion=create_instruction(path);
 	instruccion->cod=OBTENERDATOS;
 	instruccion->offset=offset;
 	instruccion->tam=size;
 	instruccion->nbytes=0;
 	return instruccion;
 }

 instruccionaMDJ* create_instruction_ca(char *path,int nbytes){
 	instruccionaMDJ *instruccion=create_instruction(path);
 	instruccion->cod=CREARARCHIVO;
 	instruccion->nbytes=nbytes;
 	return instruccion;
 }
 instruccionaMDJ* create_instruction_ba(char* path){
 	instruccionaMDJ *instruccion=create_instruction(path);
 	instruccion->cod=BORRARARCHIVO;
 	instruccion->nbytes=0;
 	return instruccion;
 }

 size_t sizecustomizado(instruccionaMDJ *instruccion){
	size_t contador=0;
 	size_t size=sizeof(operacionMDJ)+sizeof(size_t)+
			 instruccion->tamanioPath;
	 if(instruccion->cod==VALIDARARCHIVO||instruccion->cod==BORRARARCHIVO)
		 return size;
	 contador+=sizeof(instruccion->nbytes);
	 if(instruccion->cod==CREARARCHIVO)
		 return size+contador;
	 contador+=sizeof(size_t)+sizeof(off_t);
	 if(instruccion->cod==OBTENERDATOS){
		 size+=contador;
		 return size;
	 }

	 if(instruccion->cod==GUARDARDATOS){
		 size+=(contador+instruccion->tam);
		 return size;
	 }

	 else
		 return 0;

 }

 void enviarInstruccion(instruccionaMDJ *instruccion,t_log *logger,int socket){
 	 size_t contador=0;
 	 size_t size=sizecustomizado(instruccion);
 	 send(socket,(void*)&size,sizeof(size_t),0);
 	 void *buffer= malloc(size);
 	 int codi = instruccion->cod;
 	printf("ENVIANDO INFO AL MDJ:: PATH: %s :: CODIGO: %d :: TAMPATH: %d",instruccion->path,codi,instruccion->tamanioPath);
 	 memcpy(buffer,&codi,sizeof(operacionMDJ));
 	 contador+=sizeof(operacionMDJ);
 	 memcpy(buffer+contador,(void*)&instruccion->tamanioPath,sizeof(size_t));
 	 contador+=sizeof(size_t);
 	 memcpy(buffer+contador,(void*)instruccion->path,instruccion->tamanioPath);
 	 contador+=instruccion->tamanioPath;
 	 if(instruccion->cod==CREARARCHIVO){
 		memcpy(buffer+contador,(void*)&instruccion->nbytes,sizeof(instruccion->nbytes));
 	 }
 	if(instruccion->cod==OBTENERDATOS||instruccion->cod==GUARDARDATOS){
 		 memcpy(buffer+contador,(void*)&instruccion->offset,sizeof(instruccion->offset));
 		 contador+=sizeof(instruccion->offset);
 		 memcpy(buffer+contador,(void*)&instruccion->tam,sizeof(size_t));
 		 if(instruccion->cod==GUARDARDATOS){
 			 contador+=sizeof(size_t);
 			 memcpy(buffer+contador,(void*)instruccion->buf,instruccion->tam);
 		 }
 	}
 	if(buffer!=NULL)
 	send(socket,buffer,size,0);

 	free(buffer);
}
 void* recibirDevolucion(int socket_cliente,t_log *logger){
 	size_t size;
 	recv(socket_cliente,(void*)&size,sizeof(size_t),MSG_WAITALL);//MSG_WAITALL
 	int recepcionesCompletas= (int)(size/config_get_int_value(config,"TRANSFER_SIZE"));
 	int bytesACompletar= size%config_get_int_value(config,"TRANSFER_SIZE");

 	void* buffer=malloc(size);

 	int contador=0;
 	for(int j=0;j<recepcionesCompletas;j++){
 	recv(socket_cliente,buffer+j*config_get_int_value(config,"TRANSFER_SIZE"),config_get_int_value(config,"TRANSFER_SIZE"),MSG_WAITALL);
 	contador++;
 	}
 	if(bytesACompletar != 0)
 		recv(socket_cliente,buffer+contador*config_get_int_value(config,"TRANSFER_SIZE"),bytesACompletar,MSG_WAITALL);//MSG_WAITALL
 	return buffer;
 }
devMDJ* interpretarDevolucion(void* buffer){
	devMDJ *devolucion=malloc(sizeof(devMDJ));
	memcpy((void*)&devolucion->rta,buffer,sizeof(rtaaDIEGO));
	devolucion->buffer=NULL;
	devolucion->error=0;
	devolucion->tamBuffer=0;
	switch(devolucion->rta){
		case TIPOERROR:
			memcpy((void*)&devolucion->error,buffer+sizeof(rtaaDIEGO),sizeof(int));
			break;
		case DATAOBTENIDA:
			memcpy((void*)&devolucion->tamBuffer,buffer+sizeof(rtaaDIEGO),sizeof(size_t));
			devolucion->buffer=malloc(devolucion->tamBuffer);
			memcpy(devolucion->buffer,
					buffer+sizeof(rtaaDIEGO)+sizeof(size_t),
					devolucion->tamBuffer);
			break;

	}
	free(buffer);
	return devolucion;
}
devMDJ* enviarYEsperarRespuesta(instruccionaMDJ *instruccion,int socket_cliente,t_log *logger){
	enviarInstruccion(instruccion,logger,socket_cliente);
	void* buffer=recibirDevolucion(socket_cliente,logger);
	return interpretarDevolucion(buffer);
}

void crearEnFS(int i,int mdj){
	int diego = 2;
	int tamanioPayload = getHeaderSendHandshake(i,logger);
	void*paquete3 = malloc(tamanioPayload);
	log_info(logger, "el tamanio es %d", tamanioPayload);
	getPayload(paquete3, tamanioPayload, i, logger);
	int lineas;
	int dtbID;
	memcpy(&dtbID,paquete3,4);
	memcpy(&lineas, paquete3+4, 4);
	char*path3 = malloc(tamanioPayload-8);
	memcpy(path3, paquete3 + 8,tamanioPayload-8);
	diego_safa*rutina5 = malloc(sizeof(diego_safa));
	rutina5->id = dtbID;
	rutina5->memoryPointer = 0;
	rutina5->path = malloc(tamanioPayload-8);
	memcpy(rutina5->path, paquete3 + 8,tamanioPayload-8);
	printf(
			"\x1b[35mSe va a crear un archivo en %s de %d lineas\x1b[0m\n",
																	 path3, lineas);

	//comunicar mdj
	int a=1;
	send(mdj,&a,4,0);
	instruccionaMDJ* instCrear = create_instruction_ca(rutina5->path,lineas);//cambiar por all el archivo
	devMDJ* rtaDummy = enviarYEsperarRespuesta(instCrear, mdj, logger);
	if (rtaDummy->rta == TIPOERROR) {
					printf("\x1b[35mNo se pudo crear el archivo %s\x1b[0m\n",path3);
					rutina5->tipo=4;

								}
	else{
			printf("\x1b[35mSe creo el archivo\x1b[0m\n");
			rutina5->tipo=6;
		}
	printf("\x1b[35mSe creo el archivo\x1b[0m\n");
	send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
	mandarRutinaAlSafa(rutina5, safa, logger);
	//free(rutina5->path);
	free(paquete3);
	free(path3);
	free(instCrear->path);
	free(instCrear);
	free(rtaDummy->buffer);
	free(rtaDummy);
	log_info(logger,"liberados el paquete,la rutina, los path y la instruccion");
	free(rutina5);
}
void borrarEnFS(int i,int mdj){
	int diego = 2;
	int tamanioPayload = getHeaderSendHandshake(i,logger);
	void*paquete3 = malloc(tamanioPayload);
	log_info(logger, "el tamanio es %d", tamanioPayload);
	getPayload(paquete3, tamanioPayload, i, logger);
	int dtbID;
	memcpy(&dtbID,paquete3,4);
	char*path3 = malloc(tamanioPayload-4);
	memcpy(path3, paquete3 + 4,tamanioPayload-4);
	diego_safa*rutina5 = malloc(sizeof(diego_safa));
	rutina5->id = dtbID;
	rutina5->memoryPointer = 0;
	rutina5->path = malloc(tamanioPayload-4);
	memcpy(rutina5->path, paquete3 + 4,tamanioPayload-4);
	printf(
			"\x1b[35mSe va a borrar el archivo %s \x1b[0m\n",path3);

	//comunicar mdj
	int a=1;
	send(mdj,&a,4,0);
	instruccionaMDJ* instCrear = create_instruction_ba(rutina5->path);//
	devMDJ* rtaDummy = enviarYEsperarRespuesta(instCrear, mdj, logger);
			if (rtaDummy->rta == TIPOERROR) {
					printf("\x1b[35mNo se pudo crear el archivo %s\x1b[0m\n",path3);
					rutina5->tipo=4;
								}
			else{
					printf("\x1b[35mSe creo el archivo\x1b[0m\n");
					rutina5->tipo=6;
								}
	//Por ahora vamos a decir que esta siempre bien
	printf("\x1b[35mSe borro el archivo\x1b[0m\n");
	send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
	mandarRutinaAlSafa(rutina5, safa, logger);
	//free(rutina5->path);
	free(paquete3);
	free(path3);
	free(instCrear->path);
	free(instCrear);
	free(rtaDummy->buffer);
	free(rtaDummy);
	log_info(logger,"liberados el paquete,la rutina, los path y la instruccion");
	free(rutina5);
}
void abrirEnMemoria(int i,int mdj){
	int diego = 2;
	int tamanioPayload = getHeaderSendHandshake(i, logger);
	void*paquete = malloc(tamanioPayload);
	getPayload(paquete, tamanioPayload, i, logger);
	//el payload es un id de proceso y un paquete

	diego_safa*rutina2 = malloc(sizeof(diego_safa));
	rutina2->path = malloc(tamanioPayload - 4);
	//rutina2->path = malloc(tamanioPayload - 4);
	rutina2->tipo = 2; //la rutina es de tipo 2
	memcpy(&rutina2->id, paquete, 4); //id del proceso
	memcpy(rutina2->path, paquete + 4, tamanioPayload - 4); //path a abrir
	printf("\x1b[35m Abriendo %s de proceso %d\x1b[0m\n",
			rutina2->path, rutina2->id);

	free(paquete);
	printf("FREE PAQUETE");
	//pedir al mdj la lineas,meter en memoria
	int tam=0;
	char *buffer=obtenerArchivo(rutina2->path,mdj,&tam);
	buffer=realloc(buffer,tam+1);
	buffer[tam]='\0';
	//contar lineas
	if (buffer[0] == '\0') {///
		//comunicar al safa que el escriptorio no existe en el fs
		printf(
				"\x1b[35mno existe el archivo en el file system\x1b[0m\n");
		rutina2->tipo=4;
		rutina2->memoryPointer=-1;
		send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
		mandarRutinaAlSafa(rutina2, safa, logger);
		free(buffer);
		printf("FREE BUFFER");
		free(rutina2->path);
		printf("FREE PATH");
		free(rutina2);
		printf("FREE RUTINA");

		//liberar las cosas
		log_info(logger,
				"se hizo free en la inst dummy");

	} else {

		log_info(logger, "el buffer es %s",
				(char*)buffer);
		int cantLineas = contarLineas(
				(char*)buffer);
		int PID = rutina2->id;
		log_info(logger, "la cantidad de lineas son %d",
				cantLineas);
		//contar Lineas Bloqueantes
		send(fm9, &diego, 4, 0);
		void*panela = malloc(12);
		memcpy(panela, &mdj_a_fm9, 4);
		memcpy(panela + 4, &cantLineas, 4);
		memcpy(panela + 8, &PID, 4);

		send(fm9, panela, 12, 0);
		free(panela);
		printf("FREE PANELA");
		int respuestaDummyfm9 = 0;
		recv(fm9, &respuestaDummyfm9, 4, MSG_WAITALL);
		if (respuestaDummyfm9 == 0) {
			//comunicar al safa que el proceso no puede ponerse en memoria
			printf(
					"\x1b[35mel fm9 no puede guardar el archivo\x1b[0m\n");


			send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
			rutina2->tipo=4;//error
			mandarRutinaAlSafa(rutina2, safa, logger);
			//liberar las cosas


			log_info(logger,
					"se hizo free en la instAbrir path");

			log_info(logger,
					"se hizo free en la inst abrir");
			free(buffer);
			printf("FREE BUFFER");
			free(rutina2->path);
			printf("FREE PATH");
			free(rutina2);
			printf("FREE RUTINA");

		} else {

			leerArchivoYMandarLineas(buffer);

			//tendria que recibir algo para que sea standar (un int)
			//AGREGADO POR FRAN recibir del FM9 el **memoryPointer**
			int recibidoDelFM9 = -1;
			recv(fm9, &recibidoDelFM9, 4, MSG_WAITALL);
			log_info(logger,
					"el memory pointer es : %d",
					recibidoDelFM9);
			rutina2->memoryPointer=recibidoDelFM9;
			send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa
			mandarRutinaAlSafa(rutina2, safa, logger);

			log_info(logger, "la rutina del path es %s",
					rutina2->path);

			free(rutina2->path);

			free(rutina2);

			free(buffer);


		}
	}

}
void buscardummy(int buffer,int i,int mdj){
	int diego = 2;
	log_info(logger, "se recibio un %d", buffer);
	int tamanioPayload = getHeaderSendHandshake(i,
			logger);
	log_info(logger, "se recibio un tamanio de %d",
			tamanioPayload);
	void*dummyEnString = malloc(tamanioPayload);
	getPayload(dummyEnString, tamanioPayload, i,
			logger);
	log_info(logger, "El paquete contiene: %s \n",
			dummyEnString);
	DTB*dtbParaElPathYElId = malloc(sizeof(DTB));
	cargarDTB(dtbParaElPathYElId, dummyEnString,
			logger);
	free(dummyEnString);
	printf(
			"\x1b[35mEhhh, voy a buscar %s para %d\x1b[0m\n",
			dtbParaElPathYElId->path,
			dtbParaElPathYElId->ID);

	//pedirle al mdj los datos
	int tam=0;
	char*bufferDev=obtenerArchivo(dtbParaElPathYElId->path,mdj,&tam);
	bufferDev=realloc(bufferDev,tam+1);
	bufferDev[tam]='\0';

	//int a=1;
	//send(mdj,&a,4,0);

//	instruccionaMDJ* instDummy = create_instruction_od(
//			dtbParaElPathYElId->path, 0, strlen(p)+1);//HARDCODEADO PARA PRUEBA
//	devMDJ* rtaDummy = enviarYEsperarRespuesta(instDummy, mdj, logger);
	//contar lineas
	if (bufferDev[0] == '\0') {
		//comunicar al safa que el escriptorio no existe en el fs
		printf(
				"\x1b[35mno existe el archivo en el file system\x1b[0m\n");
		diego_safa*rutina = malloc(sizeof(diego_safa));
		rutina->tipo = 3; // 3 para dummy fallido
		rutina->id = dtbParaElPathYElId->ID;
		rutina->memoryPointer = 0;
		rutina->path = "";

		send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
		mandarRutinaAlSafa(rutina, safa, logger);

		//liberar las cosas
//		free(rtaDummy->buffer);
//		free(rtaDummy);
//		log_info(logger, "se hizo free en la rtadumm");
//		free(instDummy->path);
//		log_info(logger,
//				"se hizo free en la instdumm path");
//		free(instDummy);
//		log_info(logger,
//				"se hizo free en la inst dummy");

		list_destroy(
				dtbParaElPathYElId->tablaDeDirecciones);
		printf("FREE 1 1");
		free(dtbParaElPathYElId->path);
		printf("FREE 1 2");
		free(dtbParaElPathYElId);
		log_info(logger, "se hizo free en el dtb");
		printf("FREE 1 3");
		free(rutina);

	} else {

//		log_info(logger, "el buffer es %s",
//				(char*) bufferDev);

		int cantLineas = contarLineas(
				(char*) bufferDev);
		int PID = dtbParaElPathYElId->ID;
		log_info(logger, "la cantidad de lineas son %d",
				cantLineas);
		//contar Lineas Bloqueantes
		send(fm9, &diego, 4, 0);
		void*panela = malloc(12);
		memcpy(panela, &mdj_a_fm9, 4);
		memcpy(panela + 4, &cantLineas, 4);
		memcpy(panela + 8, &PID, 4);

		send(fm9, panela, 12, 0);
		free(panela);
		int respuestaDummyfm9 = 0;
		recv(fm9, &respuestaDummyfm9, 4, MSG_WAITALL);
		if (respuestaDummyfm9 == 0) {
			//comunicar al safa que el proceso no puede ponerse en memoria
			printf(
					"\x1b[35mel fm9 no puede guardar el archivo\x1b[0m\n");
			diego_safa*rutina = malloc(sizeof(diego_safa));
			rutina->tipo = 3; // 3 para dummy fallido
			rutina->id = dtbParaElPathYElId->ID;
			rutina->memoryPointer = 0;
			rutina->path = "";

			send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa al safa
			mandarRutinaAlSafa(rutina, safa, logger);

			//liberar las cosas
			printf("FREE 2 1");
			free(bufferDev);

			log_info(logger,
					"se hizo free en la rtadumm");

			list_destroy(
					dtbParaElPathYElId->tablaDeDirecciones);
			printf("FREE 2 2");
			free(dtbParaElPathYElId->path);
			printf("FREE 2 3");
			free(dtbParaElPathYElId);
			log_info(logger, "se hizo free en el dtb");
			free(rutina);

		} else {
			leerArchivoYMandarLineas(bufferDev);

			//tendria que recibir algo para que sea standar (un int)
			//AGREGADO POR FRAN recibir del FM9 el **memoryPointer**
			int recibidoDelFM9 = -1;
			recv(fm9, &recibidoDelFM9, 4, MSG_WAITALL);
			log_info(logger,
					"el memory pointer es : %d",
					recibidoDelFM9);

			diego_safa*rutina = malloc(
					sizeof(diego_safa));
			rutina->tipo = 1; // 1 para dummy
			rutina->id = dtbParaElPathYElId->ID;
			rutina->memoryPointer = recibidoDelFM9;
			rutina->path=ponerIOEnLaRutina(bufferDev);

			send(safa, &diego, 4, 0); //siempre antes de mandar cualqueir cosa
			mandarRutinaAlSafa(rutina, safa, logger);

			log_info(logger, "la rutina del path es %s",
					rutina->path);


			printf("FREE 3 1");
			free(bufferDev);
			printf("FREE 3 2");
			list_destroy(
					dtbParaElPathYElId->tablaDeDirecciones);
			printf("FREE 3 3");
			free(dtbParaElPathYElId->path);
			printf("FREE 3 4");
			free(dtbParaElPathYElId);
			log_info(logger, "se hizo free en el dtb");
			printf("FREE 3 5");
			free(rutina->path);
			printf("FREE 3 6");
			free(rutina);
		}
	}
}

void destructorDePrograma(){
config_create("asdpkasos PERRO");

}
void hacerFlush(int i,int mdj){
	int diego = 2;
	int memP;
	int accion=2;
	int ok;


	int cantLineas;
		int tamanioPayload = getHeaderSendHandshake(i,logger);
		void*paquete3 = malloc(tamanioPayload);
		log_info(logger, "el tamanio es %d", tamanioPayload);
		getPayload(paquete3, tamanioPayload, i, logger);

		int dtbID;
		memcpy(&dtbID,paquete3,4);

		//char*path3 = malloc(tamanioPayload-7);
		//memcpy(path3, paquete3 + 8,tamanioPayload-7);

		diego_safa*rutina = malloc(sizeof(diego_safa));
		rutina->id = dtbID;
		memcpy(&memP, paquete3 + 4,4);
		rutina->memoryPointer = memP;
		rutina->path = malloc(tamanioPayload-7);
		memcpy(rutina->path, paquete3 + 8,tamanioPayload-7);
		rutina->path[tamanioPayload-8]='\0';

		printf("\n\x1b[35m el paquete contiene %s \x1b[0m\n",rutina->path);
		printf("\n\x1b[35mSe va a hacer alto FLUSH \x1b[0m\n");

		//obtener Lineas de el FM9
		send(fm9,&diego,4,0);
		void* panela=malloc(12);
		memcpy(panela,&accion,4);
		memcpy(panela+4,&memP,4);
		memcpy(panela+8,&dtbID,4);
		send(fm9,panela,12,0);
		free(panela);
		//-------Recibiendo Lineas----------
		recv(fm9,&ok,4,MSG_WAITALL); //ME LLEGO 0 ACA PORQUE?
		if(ok!=1){
			log_info(logger,"el fm9 no puede mandarme lineas");//mandar al safa
			rutina->tipo=4;

		}else{
			char *buffer=string_new();
			int tamLin;
			char*baraEne="\n";
		//	char*barracero="\0";

			recv(fm9,&cantLineas,4,MSG_WAITALL);
			for(int j=0;j<cantLineas;j++){
				recv(fm9,&tamLin,4,MSG_WAITALL);
				char*linea=malloc(tamLin);
				recv(fm9,linea,tamLin,MSG_WAITALL);
				//linea[tamLin]='\0';
				string_append(&linea,baraEne);
				string_append(&buffer,linea);
				free(linea);
			}
			printf("LA INFO RECIBIDA FUE: %s \n",buffer);


		//comunicar mdj
		int a=1;
		send(mdj,&a,4,0);
		instruccionaMDJ* instborrar = create_instruction_ba(rutina->path);
		devMDJ* rtaBorrar = enviarYEsperarRespuesta(instborrar, mdj, logger);
		send(mdj,&a,4,0);
		instruccionaMDJ* instCrear = create_instruction_ca(rutina->path,cantLineas);
		devMDJ* rtaCrear = enviarYEsperarRespuesta(instCrear, mdj, logger);
		if(rtaCrear->rta==TIPOERROR){
				log_error(logger,"No se pudo crear el archivo");
				rutina->tipo=4;
				send(safa, &diego, 4, 0);
				mandarRutinaAlSafa(rutina, safa, logger);
				free(instborrar->path);
				log_info(logger,"SE HIZO FREE 3");
				free(instborrar);
				log_info(logger,"SE HIZO FREE 4");
				log_info(logger,"SE HIZO FREE 5");
				free(instCrear);
				log_info(logger,"SE HIZO FREE 6");
				free(rtaBorrar->buffer);
				log_info(logger,"SE HIZO FREE 7");
				free(rtaBorrar);
				log_info(logger,"SE HIZO FREE 8");
				free(rtaCrear->buffer);
				log_info(logger,"SE HIZO FREE 9");
				free(rtaCrear);
				//free(rutina->path);
				free(rutina);
				free(buffer);
				return;
			}
		int okay = mandarBuffer(rutina->path,mdj,buffer,strlen(buffer));//tam2
		if(okay==-1){
				log_error(logger,"No se pudo escribir en el archivo en el mdj");
				rutina->tipo=4;
				send(safa, &diego, 4, 0);
				mandarRutinaAlSafa(rutina, safa, logger);
				free(instborrar->path);
				log_info(logger,"SE HIZO FREE 3");
				free(instborrar);
				log_info(logger,"SE HIZO FREE 4");
				log_info(logger,"SE HIZO FREE 5");
				free(instCrear);
				log_info(logger,"SE HIZO FREE 6");
				free(rtaBorrar->buffer);
				log_info(logger,"SE HIZO FREE 7");
				free(rtaBorrar);
				log_info(logger,"SE HIZO FREE 8");
				free(rtaCrear->buffer);
				log_info(logger,"SE HIZO FREE 9");
				free(rtaCrear);
				log_info(logger,"SE HIZO FREE 10");
				//free(rutina->path);
				log_info(logger,"SE HIZO FREE 11");
				free(rutina);
				free(buffer);
				return;
			}

		//Por ahora vamos a decir que esta siempre bien
		printf("\x1b[35mSe realizara el flush\x1b[0m\n");

		 //siempre antes de mandar cualqueir cosa al safa

		rutina->tipo=6;
		send(safa, &diego, 4, 0);
		mandarRutinaAlSafa(rutina, safa, logger);

		free(instborrar->path);
		log_info(logger,"SE HIZO FREE 3");
		free(instborrar);
		log_info(logger,"SE HIZO FREE 4");
		log_info(logger,"SE HIZO FREE 5");
		free(instCrear);
		log_info(logger,"SE HIZO FREE 6");
		free(rtaBorrar->buffer);
		log_info(logger,"SE HIZO FREE 7");
		free(rtaBorrar);
		log_info(logger,"SE HIZO FREE 8");
		free(rtaCrear->buffer);
		log_info(logger,"SE HIZO FREE 9");
		free(rtaCrear);
		log_info(logger,"SE HIZO FREE 10LOOOOOOOOOOOOOOOL");
		free(paquete3);
		free(rutina);
		free(buffer);
		} //termina el else
}





char* ponerIOEnLaRutina(char*setLineas){

	char**lineas=string_split(setLineas,"\n");
	int size;
	int lineasBloqueantes=0;
	for (size = 0; lineas[size] != NULL; size++){
		if(esBloqueante(lineas[size]))
			lineasBloqueantes++;
	}
	log_info(logger,"LA CANTIDAD DE BLOQUEANTES SON: %d",lineasBloqueantes);
	size--;
	for(;size>=0;size--){
		free(lineas[size]);
	}
	free(lineas);
	return string_itoa(lineasBloqueantes);
}


int esBloqueante(char*linea){
	char**partesLinea=string_split(linea," ");
	int resultado=0;
	if(string_equals_ignore_case(partesLinea[0],"abrir")){
		resultado++;
	}
	if(string_equals_ignore_case(partesLinea[0],"flush")){
			resultado++;
		}
	if(string_equals_ignore_case(partesLinea[0],"crear")){
			resultado++;
		}
	if(string_equals_ignore_case(partesLinea[0],"borrar")){
			resultado++;
		}
	int size=0;
	for (size = 0; partesLinea[size] != NULL; size++);
	size--;
	for(;size>=0;size--){
		free(partesLinea[size]);
	}
	free(partesLinea);
	if(resultado==1)
		log_info(logger,"LA OPERACION ES BLOQUEANTE");

	return resultado;
}

void leerArchivoYMandarLineas(char*buffer){

	int lineasTotales=contarLineas(buffer);
	for(int i=0;i<lineasTotales;i++){
	char*linea=conseguirLineaLol(buffer,i);
	mandarLinea(fm9,linea);
	free(linea);
	}
}
char* conseguirLineaLol(char*buffer,int numeroDeLinea){
	// string_substring_until(char *text, int length);
	int contador=0;
	for(int i=0;i!=numeroDeLinea;i++){
		int tamanio=tamanioLinea(buffer+contador)+1;
		contador+=tamanio;
	}
	return string_substring_until(buffer+contador,tamanioLinea(buffer+contador)+1);
}




















