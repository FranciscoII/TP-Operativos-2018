#ifndef COMUNICACIONMDJ_H_
#define COMUNICACIONMDJ_H_

#include <stdio.h>
#include <stdlib.h>
#include "gs.h"
#include"protocolos.h"
#include <commons/string.h>

void destructorDePrograma();
char* conseguirLineaLol(char*buffer,int numeroDeLinea);
void leerArchivoYMandarLineas(char* buffer);
void comunicacionCPU();
void comunicacionMDJ();
int esBloqueante(char*linea);
char* ponerIOEnLaRutina(char*setLineas);
instruccionaMDJ* create_instruction_od(char *path,off_t offset,size_t size);
instruccionaMDJ* create_instruction_gd(char *path,off_t offset,size_t size,void* buffer);
instruccionaMDJ* create_instruction_ca(char *path,int cantidadLineas);
instruccionaMDJ* create_instruction(char* path);
instruccionaMDJ* create_instruction_va(char* path);
instruccionaMDJ* create_instruction_gd(char *path,off_t offset,size_t size,void* buffer);
void enviarInstruccion(instruccionaMDJ *instruccion,t_log *logger,int socket);
size_t sizecustomizado(instruccionaMDJ *instruccion);
void* recibirDevolucion(int socket_cliente,t_log *logger);
devMDJ* interpretarDevolucion(void* buffer);
devMDJ* enviarYEsperarRespuesta(instruccionaMDJ *instruccion,int socket_cliente,t_log *logger);
void buscardummy(int buffer,int i,int mdj);
void abrirEnMemoria(int i,int mdj);
void crearEnFS(int i,int mdj);
void hacerFlush(int i,int mdj);
void borrarEnFS(int i,int mdj);
#endif /* COMUNICACIONMDJ_H_ */
