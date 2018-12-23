#ifndef GESTIONBITARRAY_H_
#define GESTIONBITARRAY_H_
#include<stdlib.h>
#include<stdio.h>
#include<commons/bitarray.h>
#include<string.h>
#include "instrucciones.h"
#include<limits.h>
#include <unistd.h>
#include<pwd.h>
#include<sys/types.h>
	void crear_Archivo_Bitmap();
	void actualizar_Archivo_Bitmap();
	char* obtenerDatosBitArray();
	int hayBloquesDisponibles(int cantidad_bloques);
	int* obtenerIdBloquesDisponibles(int cantidad_bloques);
	int setear(int idbloque);
	int* reservarBloquesRequeridos(int cantidadBloques);
	int desSetear(int idbloque);
	void verArrayBits();
	extern int TAMBUFFER;
	extern int CANTIDADBLOQUES;


#endif /* GESTIONBITARRAY_H_ */
