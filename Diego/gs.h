#ifndef GESTION_SOCKETS_GS_H_
#define GESTION_SOCKETS_GS_H_
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<commons/log.h>
#include <commons/string.h>
#include<netdb.h>
#include<unistd.h>
#include <pthread.h>
#include <time.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#define MAX_CLIENTES 4


typedef struct{
	int id; //1999,2000 van a ser los id ejemplo
	int sizePayload;
} header;

enum estados{NEW,READY,EXEC,BLOQ,EXIT};

typedef struct {
	int estado;
	int ID;
	char*path;
	int PC;
	int FlagInic;
	int quantum;
	int memoryPointerEscriptorio;
	int IO_Bound;
	int IO_Exec;
	int sentNEW;
	t_list* tablaDeDirecciones; // segmento+tamaniopath+path
} DTB;

typedef struct{
	int tipo;
	int id;
	int memoryPointer;
	char*path;
}diego_safa;

typedef struct{
	int conexion;
	int disponibilidad;
}CPU;




//funciones para FRANCISCO entre diego y safa:

/* para esta solo declaras un struct diego_safa, lo malloqueas con un sizeof(diego_safa)
 * y le pones los 4 valores correspondientes al tipo, id,path y memory number despues la funcion hace el
 * resto
*/
void mandarRutinaAlSafa(diego_safa*rutina,int conexionSAFA,t_log*logger);

/*
 *esta funcion me rellena un diego_safa para que pueda  saber con facilidad
 *que me quiso decir el diego con eso voy a suponer tipo 1 como okey, despues
 * ya veremos los otros tipos
 */
void recibirRutinaDelDiego(diego_safa*rutina,int conexionDiego,t_log*logger);

void liberarDTBLista(void*dtb);
void liberar_lista(t_list*lista);
int tamanioTotalPaths(t_list*listaDirecciones);
void informarDTB(DTB*dtb,t_log*logger);
void cargarDTBSinInformar(DTB*dtb, void* paquete, t_log*logger);
int getHeaderNoBloqueante(int deQuien, t_log*logger);
void nuevaCPU(int servidor, CPU clientes[], int *nClientes, t_log *logger);
int dameMaximoCPU(CPU *tabla, int n);
void compactaClavesCPU (CPU *tabla, int *n);
void* dtbAPaquete(DTB*dtb);
void cargarDTB(DTB*dtb,void* paquete,t_log*logger);
int getHeaderSendHandshake(int deQuien,t_log*logger);
void getPayload(void*payload,int tamanioPayload,int deQuien,t_log*logger);
void enviarHeaderConHandshake(header*head,int aQuien,t_log*logger);
void sePudoEnviar(t_log*logger,int conexion,char*mensaje);
void enviarDTB(DTB*dtb,int aQuien,t_log*logger);
void recepcionMensaje(int receiver,t_log*logger,char*mensaje);
void enviarMensaje(int socket_cliente,t_log *logger,char *mensaje);
void recibirMensaje(int socket_cliente,t_log *logger,char *mensaje);
void recibirHeader(int socket_cliente,t_log *logger);
int connect_to_server(char * ip, char * port,t_log *logger);
int crear_server(char * ip, char * port,t_log *logger);
int esperandoUnaConexion(int socket_servidor,t_log *logger,int numConexion);
void recibir(int socket_cliente,t_log *logger,void *dato ,size_t tamTipo);
void enviar(int socket_cliente,void *dato,t_log *logger,size_t tamdetipo);
void exit_with_error(t_log *log,int socket,char *error_msg,void *buffer);
void exit_gracefully(t_log *log,int return_nr);


int dameMaximo(int *tabla, int n);
void nuevoCliente(int servidor, int *clientes, int *nClientes, t_log *logger);
void compactaClaves (int *tabla, int *n);

void esperarMili(int tiempo);
char* cargarString(void* paquete, t_log*logger);
void enviarString(char* string, int aQuien, t_log*logger);
void enviarIntconString(char* string,int num, int aQuien, t_log*logger);
#endif /* GESTION_SOCKETS_GS_H_ */
