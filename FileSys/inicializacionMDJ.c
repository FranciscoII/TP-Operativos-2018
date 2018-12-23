#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "gs.h"
#include <semaphore.h>
#include"comunicacionDAM.h"
#include "inicializacionMDJ.h"

extern t_log *logger;
extern t_config* config;
extern t_config* cmetadata;
extern int TAMBUFFER;
extern int CANTIDADBLOQUES;
extern int  TAM_BLOQUE;
extern char* DIRECTORIOACTUAL;
extern char* dirRaiz;

void inicio(){
	//cmetadata=config_create(
		//	"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz/Metadata/metadata.txt");
		cmetadata=config_create(
				"/home/utnso/workspace/fifa-entrega/Metadata/Metadata.bin");
	config =config_create("/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/MDJ.txt");
	CANTIDADBLOQUES=config_get_int_value(cmetadata,"CANTIDAD_BLOQUES");
	TAMBUFFER=(((CANTIDADBLOQUES%CHAR_BIT)==0)?(CANTIDADBLOQUES/CHAR_BIT):((CANTIDADBLOQUES/CHAR_BIT)+1));
	logger = log_create("MDJ.log", "mdj", 0,0);
	TAM_BLOQUE=config_get_int_value(cmetadata,"TAMANIO_BLOQUES");
	crear_Archivo_Bitmap();
	actualizar_Archivo_Bitmap();
	DIRECTORIOACTUAL = "/home/utnso/workspace/fifa-entrega/Archivos";
	dirRaiz="/home/utnso/workspace/fifa-entrega/Archivos";
	//crearBloques();

}



void crearBloques(){
	int cantidad=config_get_int_value(cmetadata,"CANTIDAD");
	log_info(logger,"Estoy creando los bloques!");
	char * path = "/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz/Bloques/1";
	char * p = "/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz/Bloques/";
	//char * path = "/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz/Bloques/1";
	//char * p = "/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FileSys/raiz/Bloques/";
	FILE* bloque = fopen(path,"r"); //aca se fija si existen los bloques
	if(bloque == NULL) {// @suppress("Assignment in condition")
		for(int i=0;i<cantidad;i++)
		{	char* numero_archivo = string_itoa(i);
			char* nuevoPath = malloc(strlen(p)+3);//+3 porque dos por los numero que faltan y otro para el \0
			strcpy(nuevoPath,p);
			string_append(&nuevoPath, numero_archivo);
			bloque = fopen(nuevoPath,"w");
			if(ftruncate(fileno(bloque),TAM_BLOQUE)!=0){
					log_error(logger,"No se pudo garantizar tam bloque");
				}

			fclose(bloque);
			free(nuevoPath);
		}
	return;
	}
	else
		printf("Los archivos estan creados amiguito\n");
	fclose(bloque);
}


void esperarUnaConexion(){
	int serverMDJ = crear_server("127.0.0.1",config_get_string_value(config,"PUERTO"),logger);
	int DAM = esperandoUnaConexion(serverMDJ,logger,1);
	int a=0;
	while(1){
		recv(DAM,&a,4,MSG_WAITALL);
		if(a==1)
			recibirInstruccionYAccionar(logger,DAM);
		a=0;
	}


}
