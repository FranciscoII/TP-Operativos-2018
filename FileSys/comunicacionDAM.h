#ifndef COMUNICACIONDAM_H_
#define COMUNICACIONDAM_H_
#include<stdio.h>
#include"gs.h"
#include"protocolos.h"
#include "instrucciones.h"
void ejecutarYresponder(instruccionaMDJ* instruccion,int socket_cliente);
devMDJ* ejecutarInstruccion(instruccionaMDJ *instruccion);
void enviarRespuesta(int socket_cliente,devMDJ* res);
size_t sizeCustomizadoDev(devMDJ *dev);
void* devAEnviar(devMDJ *dev);
instruccionaMDJ* interpretarInstruccion(void*buffer);
void recibirInstruccionYAccionar(t_log *logger,int socket_cliente);
#endif /* COMUNICACIONDAM_H_ */
