

#ifndef OPERACIONESDAM_H_
#define OPERACIONESDAM_H_

#include <commons/string.h>
#include <commons/config.h>
#include "gs.h"

void finalizar();
int tamanioLinea(char*buffer);
int contarLineas(char*buffer);
char*conseguirLinea(char*buffer,int tamLinea,int punteroBuffer);
void mandarLinea(int fm9,char*linea);
void liberar_lineas(char**lineas,int cantLineas);
void* obtenerArchivo(char*path,int mdj,int *s);
int mandarBuffer(char*path,int mdj,char *buffer,int size);
#endif /* OPERACIONESDAM_H_ */
