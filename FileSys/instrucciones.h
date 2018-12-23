#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<commons/string.h>
#include"protocolos.h"
#include "gestionBitarray.h"
#include"subcarpetas.h"
#include<commons/config.h>
#include<commons/log.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
extern t_config* config;
extern t_log *logger;
#define DIFICULTADESTECNICAS 99999
//---- CREARARCHIVO
#define AE 50001
#define EIA 50002
//BORRARARCHIVO
#define AIB 60001
//-----FLUSH
#define EIF 30003
#define AIF  30004
extern int  TAM_BLOQUE;

void consolaMDJ();
void recepcionesDAM();

devMDJ* validarArchivo(char *path);
char* crearCamino(char*path);
devMDJ* retornarError(int error);
devMDJ* retornarExito();
devMDJ* crearArchivo(char *path,int nbytes);
int inicializarBloques(char *path);
//size_t sizeArchivo(char* path);
int cantidadBloques(char* path);
int* obtenerBloques(char*path);
void veamoss(char *path);
char* caminoBloque(int bloque);
int garantizarLimites(char *path,size_t size, off_t offset);
int garantizoExistencia(int bloque);
void mostrarElBloque(off_t offset,size_t size,int bloque);
void* leerElBloque(off_t offset,size_t size,int bloque);
int escribirEnBloque(off_t offset,size_t size,int bloque,void *buffer);
devMDJ* guardarDatos(char *path,off_t offset,size_t size, void*buffer);
devMDJ* retornarConBuffer(void* buffer,size_t size);
devMDJ* obtenerDatos(char *path,off_t offset,size_t size);
void resetearBloque(int bloque);
void resetearBloqueHasta(int bloque,off_t offset);
int verificarValidacion(char* path);
devMDJ* borrarArchivo(char* path);
void verBloques(char *path);
void retornarMensajeError(int error);
void crearFileMetadata(char *path,int tamanio,int* bloques,int cantidadBloques);
int cantidadCifras(int n);
int cantidadBloques(char *path );
int* obtenerBloquess(char *path);
char* intarraychars(int* bloques,int cantidadDeBloques);
int agregarBloquesAlArchivo(char*path, int* newblocks,int cantnewBlocks);
int sizeArchivo(char* path);
void modificarBloques(char*path,char*bloques);
void modificar(char*path,char* tamanio,char* bloques);
void modificarTamArchivo(char*path,int tamanio);
void crearNuevo(char*path,char* tamanio,char* bloques);
int cantidadCifras(int n);
devMDJ* crearArchivo(char *path,int nbytes);
#endif /* INSTRUCCIONES_H_ */
