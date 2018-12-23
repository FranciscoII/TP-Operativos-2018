int salida =0;
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include <openssl/md5.h> // Para calcular el MD5
#include "gs.h"
#include <pthread.h>
#include <semaphore.h>
#include "instrucciones.h"
#include "consolaMDJ.h"
extern pthread_mutex_t lock;
t_config* con;
char* raiz;

void consolaMDJ() {
con=config_create("/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/MDJ.txt");
	raiz=config_get_string_value(con,"PUNTO_MONTAJE");
	chdir(raiz);
	char* directoriomuestra=getcwd(NULL,0);
	printf("DIRECTORIO ACTUAL: %s\n",directoriomuestra);
	free(directoriomuestra);
	printf("\n\x1b[92m ///////////////////////////////// \n\x1b[0m");
	printf("\n\x1b[92m Bienvenido al MDJ !!11!1!! \n \x1b[0m\n");
	printf("\n\x1b[92m ///////////////////////////////// \n\x1b[0m");
	char * linea;
	char* path;

	while (1) {
		linea = readline(" >>");
		if (linea)
			add_history(linea);
		if(!strncmp(linea,"ls ",3)){
			path = string_substring_from(linea, 3);
			string_trim(&path);
			if(!strcmp(path,"")){ //Si se escribio mal hace el if
				system("ls");
			}
			//CAMBIO
			else {
				if(dentroDeLosLimites(path)>0){
					system(linea);
				free(path);
			}


				}

		}

		if (!strcmp(linea, "ls")) {
			system("ls");
		}

		if (!strncmp(linea,"cd ",3)) {
			path = string_substring_from(linea, 3);
			string_trim(&path);  //Para poder hacer el if siguiente
			if (strcmp(path,"")==0){ //Si se escribio mal hace el if
				printf("Error, se debe incluir un parametro\n");
			}
			//CAMBIO
			else if(dentroDeLosLimites(path)>0){
				pthread_mutex_lock(&lock);
				chdir(path);
				pthread_mutex_unlock(&lock);
			}
			else
				printf("No existe el directorio leido\n");
			free(path);
		}
		if (!strncmp(linea, "md5 ",4)) {
			path = string_substring_from(linea, 4);
			string_trim(&path);
			if (!strcmp(path, "")) {
				printf("Error, se debe incluir un parametro\n");
			}
			//CAMBIO
			else{
				pthread_mutex_lock(&lock);
				int d=comprueboCaminoArchivo(path);
				if(d<0){
				printf("Archivo invalido: %d\n",d);
				free(path);
				}
				else{
					char*rutaAbsoluta= devolverRutaAbsoluta(path);
					free(path);
					char* archivo=obtenerArchivoDeLaRuta(rutaAbsoluta);
					if(tipoDeArchivo(rutaAbsoluta)==2){
						printMDJ("ARCHIVO: ");
						printf("%s \n",archivo);
						md5(archivo);
					}
					else
					printf("Vuelve a la ruta Raiz\n");
					free(rutaAbsoluta);
					free(archivo);

				}
				pthread_mutex_unlock(&lock);
			}
		}

		if (!strncmp(linea, "cat ",4)) {
			path = string_substring_from(linea, 4);
			string_trim(&path);  //Para poder hacer el if siguiente
			if (!strcmp(path, "")) { //Si se escribio mal hace el if
				printf("Debes incluir el parametro\n");
			}
			//CAMBIO
			int d=comprueboCaminoArchivo(path);
				if(d<0)
				printf("Archivo invalido: %d\n",d);
			else{
				pthread_mutex_lock(&lock);
				char*rutaAbsoluta= devolverRutaAbsoluta(path);
				char* archivo=obtenerArchivoDeLaRuta(rutaAbsoluta);
				if(tipoDeArchivo(rutaAbsoluta)==2){
				int size=sizeArchivo(archivo);
				devMDJ *dev= obtenerDatos(archivo,0,size);
				char buffer[size+1];
				memcpy(buffer,dev->buffer,size);
				free(dev->buffer);
				free(dev);
				buffer[size]='\0';
				printMDJ("\nCONTENIDO DEL ARCHIVO ");
				printf("%s",archivo);
				printMDJ(":\n\n");
				printf("%s\n",buffer);
				}
				else{
					printf("Archivo invalido: %d\n",d);
				}
				free(rutaAbsoluta);
				free(archivo);
				pthread_mutex_unlock(&lock);
			}
			free(path);
		}


				if (!strcmp(linea,"clear")) {
					pthread_mutex_lock(&lock);
					system("clear");
					pthread_mutex_unlock(&lock);

					}

					if (!strcmp(linea, "exit")) {
						free(linea);
						salida = 1;
						return;
					}
					free(linea);

	}
	free(raiz);
	config_destroy(con);
}

void printMDJ(char*linea){

printf("\x1b[92m%s\x1b[0m",linea);

}


char* irHastaContenido(char* path,int i){
			char* camino=string_substring(path,0,irHastaBarra(path,i));
			return camino;
		}

char *obtenerArchivoString(char*path){
	char* camino=string_substring(path,irHastaBarra(path,cantidadDeBarras(path))+1,strlen(path));
	return camino;
}
int comprueboCaminoArchivo(char* path){
	char*ruta=devolverRutaAbsoluta(path);
	if(ruta==NULL)
		return -2;
	int i=0;
	for(i=0;ruta[i]!=NULL;i++);
	//printf("cantidaddecaracteres: %d \n",i);
	char*rutaAbsoluta=malloc(i+1);
	memcpy(rutaAbsoluta,ruta,i);
	rutaAbsoluta[i]='\0';
	char* total=string_new();
	string_append(&total,raiz);
	string_append(&total,"/Archivos/");
	if(strncmp(rutaAbsoluta,total,strlen(total))!=0){
		free(rutaAbsoluta);
		free(ruta);
		return -1;
	}
	free(ruta);
	free(total);///////
	free(rutaAbsoluta);
	return 1;
}
char* devolverRutaAbsoluta(char* ruta){
	char *rutalimpia=realpath(ruta,NULL);
	return rutalimpia;
}
char* obtenerArchivoDeLaRuta(char *rutaAbsoluta){
	if(!rutaAbsoluta)
		return NULL;
	int i=0;
	for(i=0;rutaAbsoluta[i]!=NULL;i++);
	char* ruta=malloc(i+1);
	memcpy(ruta,rutaAbsoluta,i);
	ruta[i]='\0';
	char* total=string_new();
	string_append(&total,raiz);
	string_append(&total,"/Archivos");
	int anterior=strlen(total);
	char*archivo=string_substring_from(ruta, anterior);
	free(ruta);
	free(total);
	return archivo;
}

int tipoDeArchivo(char *name){
	struct stat date;
	int respuesta=-2;
	if(stat(name,&date)==-1)
		respuesta= -1;
	if(S_ISDIR(date.st_mode))
		respuesta=1;
	if(S_ISREG(date.st_mode))
		respuesta= 2;
	return respuesta;
}
int cantidadDeBarrass(char* path){
	int i,contador;
	contador=0;
	for(i=0;i<strlen(path);i++)
		if(path[i]=='/')
			contador++;
	return contador;
}
int irHastaBarra(char* path,int i){
	int c=0;
	while((i>0)&&(c<strlen(path))){
			if(path[c]=='/')
				i--;
			c++;
		}
		return c-1;

}
int dentroDeLosLimites(char* ruta){
	char* absoluta=devolverRutaAbsoluta(ruta);
	if(!absoluta)
	return -2;
	int a=-1;
	a= string_contains(absoluta,raiz);
	free(absoluta);
	return a;
}


void md5(char*archivo){
	int size=sizeArchivo(archivo);
	devMDJ *dev= obtenerDatos(archivo,0,size);
	char bufferc[size+1];
	memcpy(bufferc,dev->buffer,size);
	free(dev->buffer);
	free(dev);
	bufferc[size]='\0';
	unsigned char * digest = malloc(MD5_DIGEST_LENGTH);
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context,bufferc,size);
	MD5_Final(digest, &context);
	int i;
	printMDJ(" \n el MD5 es: ");
	for (i=0;i<MD5_DIGEST_LENGTH;i++)
		printf("%02x",digest[i]);
	printf("\n");
	free(digest);

}
//primeros intentos

/*
char* devolverRutaAbsoluta(char *path){
	char* rutaAbsoluta=string_new();
	char*actualpath;
	char *rutaAbsolutaAnterior=getcwd(NULL,0);
	if(path[0]!='/'){
			int i;
			for(i=0;rutaAbsolutaAnterior[i]!=NULL;i++);
			realloc(rutaAbsolutaAnterior,i+1);
			rutaAbsolutaAnterior[i]='\0';
			printf("rutaAbsoluta anterior: %s \n",rutaAbsolutaAnterior);
			rutaAbsoluta=string_new();
			string_append(&rutaAbsoluta,rutaAbsolutaAnterior);
			string_append(&rutaAbsoluta,"/");
			string_append(&rutaAbsoluta,path);
		}
		else{
			rutaAbsoluta=string_new();
			string_append(&rutaAbsoluta,path);
		}
	printf("rutaAbsoluta %s \n",rutaAbsoluta);
	if(tipoDeArchivo(rutaAbsoluta)==1){
		chdir(path);
		free(rutaAbsoluta);
		rutaAbsoluta=getcwd(NULL,0);
		int j;
		for(j=0;rutaAbsoluta[j]!=NULL;j++);
			realloc(rutaAbsoluta,j+1);
			rutaAbsoluta[j]='\0';
	}
	if(tipoDeArchivo(rutaAbsoluta)==2){
		printf("\nEn camino 2\n");
		int c=cantidadDeBarrass(rutaAbsoluta);
		char* camino=irHastaContenido(rutaAbsoluta,c);
		printf("\n CAMINO: %s\n",camino);
		chdir(camino);
		free(rutaAbsoluta);
		rutaAbsoluta=getcwd(NULL,0);
		int j;
		for(j=0;rutaAbsoluta[j]!=NULL;j++);
		realloc(rutaAbsoluta,j+2);
		rutaAbsoluta[j]='\0';
		printf("Ruta absoluta sin archivo: %s\n\n",rutaAbsoluta);
		char*archivo=obtenerArchivoString(path);
		printf("ARCHIVO: %s\n",archivo);
		string_append(rutaAbsoluta,archivo);
		free(archivo);
		free(camino);
	}
	else if(tipoDeArchivo(rutaAbsoluta)<0){
		chdir(rutaAbsolutaAnterior);
		free(rutaAbsolutaAnterior);
		return NULL;
	}

	chdir(rutaAbsolutaAnterior);
	free(rutaAbsolutaAnterior);
	return rutaAbsoluta;
}
*/
//PRUEBA DE QUE LA FUNCION PROBAR CAMINO ANDA
/*	char* a=devolverRutaAbsoluta("Archivos/../Archivos/scripts");
	printf("1.a %s \n",a);
	free(a);
	printf("1.b %d \n",comprueboCaminoArchivo("Archivos/../Archivos/scripts"));
	a=devolverRutaAbsoluta("Archivos/../Archivos/scripts");
	printf("2.a %s \n",a);
	free(a);
	printf("2.b %d \n",comprueboCaminoArchivo("Archivos/../Archivos/scripts"));
	a=devolverRutaAbsoluta("Archivos/scripts/../../Archivos/scripts/simple.escriptorio");
	printf("3.a %s \n",a);
	free(a);
	printf("3.b %d \n",comprueboCaminoArchivo("Archivos/scripts/../../Archivos/scripts/simple.escriptorio"));
	a=devolverRutaAbsoluta("Archivos/scripts/../../Archivos/scripts/simple.escriptorigfdo");
	printf("4.a %s \n",a);
	free(a);
	printf("4.b %d \n",comprueboCaminoArchivo("Archivos/scripts/../../Archivos/scripts/simple.escriptorgfdgio"));
*/
