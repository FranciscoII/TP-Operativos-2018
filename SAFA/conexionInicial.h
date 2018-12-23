

#ifndef CONEXIONINICIAL_H_
#define CONEXIONINICIAL_H_

#include "gs.h"
#include <commons/log.h>
#include <commons/string.h>


//si se realizaron estas 2 conexiones: 1 si, 0 no

int sePudieronConectar(int conexion1,int conexion2,t_log*logger);

//si un buffer es DAM y el otro es CPU. 1 si, 0 no

int sonElDamYunCpu(char*bufferId1,char*bufferId2, t_log* logger);

//si 2 conexiones se realizaron y una proviene de un cpu y otra del diego
// devuelve el numero de conexion de diego o 0 porque algo fallo

int seConectaronElDamYUnCpu(int conexion1,int conexion2,char*bufferId1,char*bufferId2,t_log*logger);

//segun la validacion del safa devuelve quien es el diego
int soyElDiego(int validacion,int conexion1,int conexion2);

//segun la validacion del safa devuelve quien es el cpu
int soyElCpu(int validacion,int conexion1,int conexion2);

#endif /* CONEXIONINICIAL_H_ */
