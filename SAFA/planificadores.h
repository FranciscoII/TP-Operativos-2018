/*
 * planificadores.h
 *
 *  Created on: 23 sep. 2018
 *      Author: utnso
 */
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include "gs.h"
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <commons/config.h>
#define MAX_CPU 10
extern t_config*config;
typedef struct{
	char*nombre;
	int proceso;
	int instancias;
	int procesosEnCola[50];
}recurso;


#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

void liberarProcesos();
void cambiarRetardo(char*nuevoRetardo);
void aniadirAColaDeTiempos(time_t tiempoEnvio,time_t tiempoLlegada);
void cerrarEnMemoria(DTB*dtb);
void actualizarNEW(int sentenciasNEW);
void planificarDUMMY(DTB*dtbAMandar);
void llenarDTBYPonerEnReady(diego_safa* rutina);
void cambiarGradoMP(char*nuevoGrado);
void cambiarQuantum(char*nuevoQuantum);
void cambiarAlgoritmo(char*nuevoAlgoritmo);
void cargarDTBConMasIOYBorrar(DTB*dtb);
void status();
void*conseguirProcesoEnPRIORIDAD(int proceso,t_log*logger);
void destruirProcesoEnPRIORIDAD(int id,t_log*logger);
int estaElProcesoEnPRIORIDAD(int id, int*pos,t_log*logger);
void actualizarRecursos();
void moverLaCola(recurso*resource);
void otorgarRecurso(recurso* resource);
void crearRecurso(char *nombreRec);
void liberarOCrearRecurso(int proceso,char*nombreRec);
void crearRecursoYOtorgar(int proceso,char *nombreRec);
int intentarOtorgar(int proceso,recurso*resource);
int buscarRecursoYOtorgar(int proceso,char* recurso);
void operacionSignal(int socket);
void operacionWait(int socket);
void*conseguirProcesoEnEXIT(int proceso,t_log*logger);
int estaElProcesoEnEXIT(int id, int*pos,t_log*logger);
void*conseguirProcesoEnNEW(int proceso,t_log*logger);
double promedioEXIT();
void mostrarSentenciasNEW(DTB*dtb);
void*conseguirProcesoEnREADY(int proceso,t_log*logger);
void mostrarMetricasDTB(char* idDTB);
void*conseguirProcesoEnEXEC(int proceso,t_log*logger);
void mostrarDatosSistema(float ejecPromDAM,int porcentageDam,double tiempoRespuesta,float promEXIT);
void mostrarMetricasSistema();
void destruirProcesoEnBLOQ(int id,t_log*logger);
void destruirProcesoEnNEW(int id,t_log*logger);
void moverSegunAlgoritmo(char*algoritmo,DTB*dtb,int posicion);
void destruirProcesoEnREADY(int id,t_log*logger);
void finalizar(char* idDTB,t_log*logger);
void mostrarDTB(DTB*dtb);
int estaElProcesoEnNEW(int id, int*pos,t_log*logger);
int estaElProcesoEnREADY(int id, int*pos,t_log*logger);
void mostrarDatosDTB(char* idDTB,t_log*logger);
void informarCola(t_list* cola);
void destruirProcesoEnEXEC(int id,t_log*logger);
int estaElProcesoEnEXEC(int id, int*pos,t_log*logger);
void printSafa(char*palabras);
void*conseguirProcesoEnBLOQ(int proceso,t_log*logger);
int estaElProcesoEnBLOQ(int id, int*pos,t_log*logger);
void desbloquearProceso(int id,t_log*logger,diego_safa*rutina,char*algoritmo,int posicion);
void conexiones();
void planificar(DTB*dtb, char* algoritmo, t_log*logger);
void enviarDTBACPU(DTB*dtbAMandar,t_log*logger);
void PrimerasConexiones();
void consola();
void plp();
void plp_cola();
void pcp();
void pcp_NEWtoREADY();
void diego_com();



#endif /* PLANIFICADORES_H_ */
