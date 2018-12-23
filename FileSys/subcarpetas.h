#ifndef SUBCARPETAS_H_
#define SUBCARPETAS_H_
#include<stdio.h>
#include<commons/config.h>
#include<commons/string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
char *obtenerArchivo(char*path);
char* irHastaContenidoN(char* path,int i);
int cantidadDeBarras(char* path);
char* pathCompleto(char *path);
void crearSubcarpetas(char*path);



#endif /* SUBCARPETAS_H_ */
