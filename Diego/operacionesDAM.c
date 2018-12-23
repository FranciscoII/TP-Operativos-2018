#include "operacionesDAM.h"
#include"comunicacionMDJ.h"
extern t_config* config;
extern t_log* logger;

void liberar_lineas(char**lineas,int cantLineas){

	for(int i=cantLineas;i<0;i--){
	free(lineas[i]);
	}
	free(lineas);
}


int contarLineas(char*buffer){
	int contadorPalabras=0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i]=='\n')
			contadorPalabras++;
	}
	return contadorPalabras;
}
int tamanioLinea(char*buffer){
	int contador=0;

	for(int i=0;buffer[i]!='\n' ;i++){
		contador++;
		}
	return contador;
}
char*conseguirLinea(char*buffer,int tamLinea,int punteroBuffer){
	char*bufferAux=malloc(tamLinea);
	memcpy(bufferAux,buffer+punteroBuffer,tamLinea);
	return bufferAux;
}
void mandarLinea(int fm9,char*linea){
	void*paquete=malloc(strlen(linea)+1+4);
	int transferSize=config_get_int_value(config,"TRANSFER_SIZE");
	int tamanioPaquete=strlen(linea)+1+4;
	int tamanioLinea=strlen(linea)+1;
	memcpy(paquete,&tamanioLinea,4);
	memcpy(paquete+4,(void*)linea,tamanioLinea);

	int tamanioAEnviar;
	if(transferSize>tamanioPaquete){
		tamanioAEnviar=tamanioPaquete;
	}else{
		tamanioAEnviar=transferSize;
	}

	int ciclosCompletos= tamanioPaquete/transferSize;
	int cicloIncompleto=tamanioPaquete%transferSize;
	int contador=0;
	for(int i=0;i<ciclosCompletos;i++){
		send(fm9,paquete+i*transferSize,tamanioAEnviar,0);
		contador++;
	}
	send(fm9,paquete+contador*transferSize,cicloIncompleto,0);
	free(paquete);
}


void* obtenerArchivo(char*path,int mdj,int *s){
	int a=1;
	int transferSize=config_get_int_value(config,"TRANSFER_SIZE");
	int desplazamiento=0;
	char *buffer=string_new();
	int contador=0;
	while(a>0){
	        send(mdj,&a,4,0);
	        instruccionaMDJ* instruccion= create_instruction_od(path,desplazamiento,transferSize);
	        devMDJ* dev =enviarYEsperarRespuesta(instruccion,mdj,logger);
	        if((dev->rta)==TIPOERROR){
	            log_error(logger,"No se pudieron obtener datos desde el mdj\n");
	            free(instruccion);
	            free(dev->buffer);
	            free(dev);
	            free(buffer);
	            return NULL;
	        }
	        int tam=(dev->tamBuffer);
	        printf("tam %d\n", tam);
	        log_info(logger,"el tamanio recibido fue %d",tam);
	        if(tam==0){
	        	free(instruccion);
	            free(dev->buffer);
	        	free(dev);
	        	return buffer;
	        }
	        contador+=tam;
	        int tam2=desplazamiento+tam;
	        buffer=realloc(buffer,tam2);
	        //prueba de lo que llega
	        memcpy(buffer+desplazamiento,dev->buffer,tam);
	        if(tam<transferSize){
	            a=0;
	            log_info(logger,"Terminada la transferencia de datos desde MDJ");
	        }
	        desplazamiento+=transferSize;
	        free(instruccion);
	        free(dev->buffer);
	        free(dev);
	        log_info(logger,"se hizo free en la instruccion");
	        }


	*s=contador;
	return buffer;
}

int mandarBuffer(char*path,int mdj,char *bufferEnString,int size){
	int a=1;
	int transferSize=config_get_int_value(config,"TRANSFER_SIZE");
	int desplazamiento=0;
	int tam=transferSize;
	void *buffer=malloc(size);
	memcpy(buffer,bufferEnString,size);

	while(a>0){
		int b=1;
		if(size<transferSize){
			tam=size;
			a=0;
		}
		else
		size-=transferSize;
		send(mdj,&b,4,0);

		log_info(logger,"se van a enviar al mdj %d bytes",tam);
		log_info(logger,"restan enviar %d",size);
		instruccionaMDJ* instruccion= create_instruction_gd(path,desplazamiento,tam,buffer+desplazamiento);
		devMDJ* dev =enviarYEsperarRespuesta(instruccion,mdj,logger);
		if((dev->rta)==TIPOERROR){
					log_error(logger,"No se pudieron obtener datos desde el mdj\n");
					free(instruccion->buf);
					free(instruccion);
					free(dev);
					free(buffer);
					return -1;
				}
		desplazamiento+=transferSize;
		free(instruccion->buf);
		free(instruccion);
		free(dev);

		}
	free(buffer);
	return 1;
}



