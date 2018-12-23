#include"comunicacionDAM.h"
#include <commons/log.h>
#define INEXISTENTE -99999
extern t_log* logger;
extern t_config* config;

devMDJ* ejecutarInstruccion(instruccionaMDJ *instruccion){
	devMDJ *res;
	switch(instruccion->cod){
	case VALIDARARCHIVO:
		log_info(logger,"Se esta por validar el archivo %s",instruccion->path);
		res=validarArchivo(instruccion->path);
		break;
	case BORRARARCHIVO:
		log_info(logger,"Se esta por borrar el archivo %s",instruccion->path);
		res=borrarArchivo(instruccion->path);
		break;
	case CREARARCHIVO:
		log_info(logger,"Se esta por crear el archivo %s",instruccion->path);
		res=crearArchivo(instruccion->path,instruccion->nbytes);
		break;
	case OBTENERDATOS:
		log_info(logger,"Se esta por obtener datos del archivo %s",instruccion->path);
		res=obtenerDatos(instruccion->path,instruccion->offset,instruccion->tam);
		break;
	case GUARDARDATOS:
		log_info(logger,"Se esta por guardar datos del archivo %s",instruccion->path);
		res=guardarDatos(instruccion->path,instruccion->offset,instruccion->tam,instruccion->buf);
		break;
	}
	return res;

}
void ejecutarYresponder(instruccionaMDJ* instruccion,int socket_cliente){
	devMDJ *res=ejecutarInstruccion(instruccion);
	enviarRespuesta(socket_cliente,res);
	return;
}
void enviarRespuesta(int socket_cliente,devMDJ* res){
	size_t size;
	void* buffer=devAEnviar(res);
	memcpy((void*)&size,buffer,sizeof(size_t));
	send(socket_cliente,(void*)&size,sizeof(size_t),0);
	send(socket_cliente,buffer+sizeof(size_t),size,0);
	free(buffer); //AACA CAMBIE
	free(res->buffer);//ACA
	free(res);

}
size_t sizeCustomizadoDev(devMDJ *dev){
	size_t size=sizeof(rtaaDIEGO);
	if(dev->rta==TIPOERROR)
		size+=sizeof(int);
	if(dev->rta==DATAOBTENIDA){
		size+=sizeof(size_t);
		size+=dev->tamBuffer;
	}
		return size;
}
void* devAEnviar(devMDJ *dev){
	log_info(logger,"Le respondo al diego");
	size_t size=sizeCustomizadoDev(dev);
	void* buffer=malloc(size+sizeof(size_t));
	memcpy(buffer,(void*)&size,sizeof(size_t));
	memcpy(buffer+sizeof(size_t),(void*)&dev->rta,sizeof(rtaaDIEGO));
	switch(dev->rta){
	case DATAOBTENIDA:
		memcpy(buffer+sizeof(size_t)+sizeof(rtaaDIEGO),(void*)&dev->tamBuffer,sizeof(size_t));
		memcpy(buffer+sizeof(size_t)*2+sizeof(rtaaDIEGO),dev->buffer,dev->tamBuffer);
		break;
	case TIPOERROR:
		memcpy(buffer+sizeof(size_t)+sizeof(rtaaDIEGO),(void*)&dev->error,sizeof(int));
		break;
	}
	return buffer;
	}


instruccionaMDJ* interpretarInstruccion(void*buffer){
	 size_t contador=0;
	 instruccionaMDJ*instruccion=malloc(sizeof(instruccionaMDJ));
	 memcpy((void*)&instruccion->cod,buffer,sizeof(operacionMDJ));
	 //printf("CODIGO ES: %d\n",instruccion->cod);
	 contador+=sizeof(operacionMDJ);
	 memcpy((void*)&instruccion->tamanioPath,buffer+contador,sizeof(size_t));
	 contador+=sizeof(size_t);
	 //printf("TAMPATH ES: %d\n",instruccion->tamanioPath);
	 instruccion->path=malloc(instruccion->tamanioPath);
	 memcpy((void*)instruccion->path,buffer+contador,instruccion->tamanioPath);
	 //printf("PATH ES: %s\n",instruccion->path); // @suppress("Field cannot be resolved")
	 contador+=instruccion->tamanioPath;
	 if(instruccion->cod==CREARARCHIVO){
		memcpy((void*)&instruccion->nbytes,buffer+contador,sizeof(instruccion->nbytes));
	 }
	if(instruccion->cod==OBTENERDATOS||instruccion->cod==GUARDARDATOS){
		 memcpy((void*)&instruccion->offset,buffer+contador,sizeof(off_t));
		 contador+=sizeof(instruccion->offset);
		 memcpy((void*)&instruccion->tam,buffer+contador,sizeof(size_t));
		 if(instruccion->cod==GUARDARDATOS){
			 contador+=sizeof(size_t);
			 instruccion->buf=malloc(instruccion->tam);
			 memcpy(instruccion->buf,buffer+contador,instruccion->tam);
			 //printf("TAMAÑO BUFFER &d", instruccion->tam);
		 	 }
	}
	return instruccion;

}

void  recibirInstruccionYAccionar(t_log *logger,int socket_cliente){
	log_info(logger,"Se esta recibiendo una instruccion del diego ");
	size_t tamanio;
	recv(socket_cliente,(void*)&tamanio,sizeof(size_t),MSG_WAITALL);
	//printf("Hola estoy aca en recibirYAccionar\n");
	//printf("%ld\n",tamanio);
	void* buffer=malloc(tamanio);
	recv(socket_cliente,buffer,tamanio,MSG_WAITALL);
	instruccionaMDJ *instruccion=interpretarInstruccion(buffer);
	ejecutarYresponder(instruccion,socket_cliente);
	config_destroy(config);
	free(instruccion->path); //ACA
	if(instruccion->cod==GUARDARDATOS)
		free(instruccion->buf);
	free(instruccion);
	free(buffer);//ACA CAMBIE
	config =config_create("/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/MDJ.txt"); //El logger se destruye, asi que se vuelve a crear!
	return;

}


