#include "funcionesFM9.h"
extern t_list* tablaSegmentos;
extern t_log* loggerFM9;
extern int tam_max_linea;
extern int tam_max_pag;
extern int tam_max_pag;
t_list* tablaPagAuxiliar;


int encontrarMarco(int seg, int offset) {
	int tamLineasPag = tam_max_pag / tam_max_linea;
	int nroPagina = offset / tamLineasPag;
	offset = offset % tamLineasPag;
	t_segmentoSEGPAG* segBusc;
	t_paginaSEGPAG* pagBusc;
	//printf("Marco de la primer pagina del seg %d\n",pagBusc->marco);
	int tamTabla = list_size(tablaSegmentos);
	for (int i = 0; i < tamTabla; i++) {
		segBusc = list_get(tablaSegmentos, i);
		if (seg == segBusc->id) {
			for (int j = 0; j < list_size(segBusc->tabla); j++) {
			//	printf("buscando vuelta nro %d\n", j);
				pagBusc = list_get(segBusc->tabla, j);
			//	printf("NUMERO DE PAGINA %d\n", pagBusc->nroPag);
				if (nroPagina == pagBusc->nroPag)
					return pagBusc->marco;
			}
		}
	}
	return -1;
}

int obtenerPrimerMarcoLibre() {
	//int tamLineasPag =tam_max_pag/tam_max_linea;
	int marco = obtenerBaseParaSegDeNLineas(1);
	return marco;
}

void agregarPagALista(int base, int page) {
	t_paginaSEGPAG* pagina = malloc(sizeof(t_paginaSEGPAG));
	pagina->marco = base;
	pagina->nroPag = page;
	list_add(tablaPagAuxiliar, pagina);
	printf("numero de pagina %d\n", pagina->nroPag);
	return;
}

int encontrarLimite(int pid, int id){//EN ESTA FUNCION FALTA UN FREE CREO DE SEG BUSCADO
	t_segmentoSEGPAG* segBuscado;// = malloc(sizeof(t_segmentoSEGPAG));
	int limBusc;
	int encontrado = 0;
	for (int i = 0; i < list_size(tablaSegmentos); i++) {
				segBuscado = list_get(tablaSegmentos, i);
				if (segBuscado->PID == pid && segBuscado->id == id){
					limBusc = segBuscado->limite;
					encontrado = 1;
				}
	}
	if(encontrado)
		return limBusc;
	else
		return -1;
}

void liberar_tablaSegmentosPag(t_list*lista) {
	log_info(loggerFM9,"Liberando Tabla Segmentos");
	for (int i = list_size(lista) - 1; i >= 0; i--) {
		list_remove_and_destroy_element(lista, i, (void*) liberarSegmentosPag);
	}
}
void liberarSegmentosPag(t_segmentoSEGPAG*nodo) {
	log_info(loggerFM9,"Liberando Tabla SegmentosPag");
	liberar_tablaPag(nodo->tabla);
	list_destroy(nodo->tabla);//ACA CAMBIE
	free(nodo);
}
void liberar_tablaPag(t_list*lista) {
	log_info(loggerFM9,"Liberando Tabla Pagina del segmento");
	for (int i = list_size(lista) - 1; i >= 0; i--) {
		list_remove_and_destroy_element(lista, i, (void*) liberarPags);
	}
}
void liberarPags(t_paginaSEGPAG*paquete) {
	free(paquete);
}
