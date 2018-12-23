#ifndef FUNCIONESFM9_H_
#define FUNCIONESFM9_H_

#define CANTIDADLINEAS 32 //Por ahora es asi
#define TAMBUFFER  (((CANTIDADLINEAS%CHAR_BIT)==0)?(CANTIDADLINEAS/CHAR_BIT):((CANTIDADLINEAS/CHAR_BIT)+1))
#define FLUSH -2
#include "gs.h"
#include<commons/bitarray.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<string.h>
#include<commons/collections/list.h>
#include <semaphore.h>
enum modos{SEG, PAG, SEGPAG};
typedef struct{
	int id;
	int limite;
	int base;
	int PID;
} t_segmentacion;

typedef struct{
	int indice;
	int PID;
	int pagina;
}t_TablaINV;

typedef struct{
	int marco;
	int lineasOcupadas;
	int terminaUnArchivo;
}t_TI_InfoAux;

typedef struct{
	int id;
	int PID;
	int limite;
	t_list * tabla;
}t_segmentoSEGPAG;

typedef struct{
	int nroPag;
	int marco;
}t_paginaSEGPAG;

	void consola();
	void crear_Archivo_Bitmap();
	char* obtenerDatos();
	int hayBloquesDisponibles(int cantidad_bloques);
	int* obtenerIdBloquesDisponibles(int cantidad_bloques);
	int setear(int idbloque);
	int* reservarBloquesRequeridos(int cantidadBloques);
	int desSetear(int idbloque);
	void verArrayBits();
	int obtenerBaseParaSegDeNLineas(int cantidad_Lineas);
	int obtenerPrimerMarcoLibre();
	void coms(void);
	int buscarLineaPedida(int pid, int SoP, int offset, int procesador);
	void recibirYGuardarLineas(int socketDiego,int nro,int base,int ID);
	int estaEnMemoria(int memPoint,int PID);
	int enviarMemoriaADiego(int diego,void* paquete);
	void cambiarPosicion(int pid, int SoP, int offset, char* datos,int cpuPiola);
	void liberarMemoria(int pid,int SoP);

	int realizarAccion(void* paquete);
	void actualizarListas(int base,void* paquete);
	int encontrarBaseSegmento(int id);
	int encontrarLimiteSegmento(int id);
	int encontrarMarco(int seg,int offset);
	void agregarPagALista(int base,int pag);
	int encontrarLimite(int pid, int id);

	int TI_devolverLinea(int marco, int offset, int tamanioTabla, int tamanioLinea,
			int tamanioPag, t_list* tablaInfo,t_list* tablaINV, t_log* logger, char** linea);
	int TI_agregarLinea(int ID, char* linea, t_list* tablaInfo, t_list* tablaINV,
			int tamPag, int tamanioTabla, int tamLinea, t_log* logger, int marco);
	int TI_devolverMarcoVacioUsableParaProceso(int ID, t_list* tablaINV,
			t_list* tablaInfo, int tamanioTabla, int tamPag, int tamLinea);
	int TI_marcoEstaLleno(int marco, t_list* tablaInfo, int tamanioTabla,
			int tamPag, int tamLinea);
	int TI_buscarMarco(int ID, int PAG, t_list* tabla, int tamanioTabla);
	int TI_agregarLineaPrimera(int ID, char* linea, t_list* tablaInfo, t_list* tablaINV,
			int tamPag, int tamanioTabla, int tamLinea, t_log* logger);
	void TI_removerPagina(int ID, int pagina);
	int TI_modificarLinea(int marco, int offset, char* linea, int tamanioTabla,
			int tamanioLinea, int tamanioPag, t_list*tablaInfo,t_list*tablaINV, t_log* logger);
	int TI_proximaPagina(int ID, int pagina);
	void TI_enviarPaginaAlDiego(int ID, int pagina, int diego);
	int TI_lineasEnMemoriaDeArchivo(int marcoInicial);
	void TI_mostrarEstadisticas();
	int TI_lineasLibresEnMemoria();
	void liberar_tablaInvertida(t_list*tablaINV, t_list*tablaINFO ,int tamanioTabla);
	int liberarStructuraAdministrativa(int PID,int SoP);

	void inicializarSegmentacion(void);
	void administracionSegmentacion();
	int devolverModo(t_config* config);
	void inicializarTablaInvertida();
	void inicializarSegmentacionPaginada();
	void liberar_tablaSegmentos(t_list*lista);
	void liberarSegmentos(t_segmentacion*paquete);
	void liberar_tablaPag(t_list*lista);
	void liberarPags(t_paginaSEGPAG*paquete);
	void liberar_tablaSegmentosPag(t_list*lista);
	void liberarSegmentosPag(t_segmentoSEGPAG*paquete);
	void informarLista();

	void mandarLineaADiego(int diego,char*linea);
	void printearLineas(int pid, int id);
#endif /* FUNCIONESFM9_H_ */
