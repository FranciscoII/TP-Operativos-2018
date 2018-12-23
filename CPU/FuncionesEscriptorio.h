#ifndef FUNCIONESESCRIPTORIO_H_
#define FUNCIONESESCRIPTORIO_H_

#include "gs.h"
#include"protocolos.h"

int abrir(char* linea,DTB*dtb);
int asignar(char* linea,DTB*dtb);
int wait(char* linea,DTB*dtb);
int signal(char* linea,DTB*dtb);
void closeEscriptorio(DTB*dtb);
int closeTP(char* linea,DTB*dtb);
int flush(char* linea, DTB*dtb);
int crear(char* linea,DTB*dtb);
int borrar(char* linea,DTB*dtb);

int obtenerMemoryPointer(t_list* tabla,char* path);
#endif /* FUNCIONESESCRIPTORIO_H_ */
