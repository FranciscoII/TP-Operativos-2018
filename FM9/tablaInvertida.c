#include "funcionesFM9.h"

extern char* storage;
extern int tam_max_pag;
extern int tam_max_linea;
extern t_list* TI_tablaInfoAuxilar;
extern t_list* tablaInvertida;
extern int tamanioTabla;
extern t_log* loggerFM9;

int TI_buscarMarco(int ID, int PAG, t_list* tabla, int tamanioTabla) //TI = Tabla Invertida
{ //Dado un PID y una pagina devuelve el marco asignado a esa pagina

	for (int i = 0; i < tamanioTabla; i++) {

		t_TablaINV* elemento = list_get(tabla, i);

		if (elemento->PID == ID) {
			if (elemento->pagina == PAG) {
				int indice = elemento->indice;
				return indice;
			}
		}
	}
	return -1; //No se encontro
}

int TI_devolverOffsetVacioDeMarco(int marco, t_list* tablaInfo,
		int tamanioTabla) { //Busca un espacio vacio dentro de una pagina

	for (int i = 0; i < tamanioTabla; i++) {
		t_TI_InfoAux* elemento = list_get(tablaInfo, i);

		if (elemento->marco == marco) {
			int offset = elemento->lineasOcupadas;
			return offset;
		}
	}
	return -1; //No se encontro
}

int TI_buscarPaginaMasAlta(int ID, t_list* tabla, int tamanioTabla) //Devuelve el numero mas alto de pagina que tiene un proceso
{
	int max = -1;

	for (int i = 0; i < tamanioTabla; i++) { //Busca el numero mas alto

		t_TablaINV* elemento = list_get(tabla, i);

		if (elemento->PID == ID) {
			if (elemento->pagina > max) {
				max = elemento->pagina;
			}
		}
	}
	return max;
}

int TI_asignarMarcoAProceso(int ID, t_list* tabla, int tamanioTabla) {

	for (int indice = 0; indice < tamanioTabla; indice++) {
		t_TablaINV* elemento = list_get(tabla, indice);

		if (elemento->PID == -1) //Se encontro un marco no usado
				{
			elemento->PID = ID;
			elemento->pagina = 1
					+ TI_buscarPaginaMasAlta(ID, tabla, tamanioTabla); //Se asigna numero de pagina, que sera el siguiente al mayor numero de pagina

			return elemento->indice; //Se devuelve el marco, de yapa
		}

	}
	return -1; //No hay marcos disponibles
}

int TI_marcoEstaLleno(int marco, t_list* tablaInfo, int tamanioTabla,
		int tamPag, int tamLinea) {
	int ultimaLinea = TI_devolverOffsetVacioDeMarco(marco, tablaInfo,
			tamanioTabla);

	if (ultimaLinea < (tamPag / tamLinea)) {
		return 0;
	}

	return 1;
}

int TI_marcoEstaVacio(int marco, t_list* tablaInfo, int tamanioTabla,
		int tamPag, int tamLinea) {
	int ultimaLinea = TI_devolverOffsetVacioDeMarco(marco, tablaInfo,
			tamanioTabla);

	if (ultimaLinea != 0) {
		return 0;
	}

	return 1;
}

int TI_devolverMarcoUsableParaProceso(int ID, t_list* tablaINV,
		t_list* tablaInfo, int tamanioTabla, int tamPag, int tamLinea) { //Devuelve un marco donde el proceso "ID" pueda escribir una nueva linea

	int pagina = TI_buscarPaginaMasAlta(ID, tablaINV, tamanioTabla);

	if (pagina == -1) { //Osea si no tenia ninguna pagina en memoria
		if (TI_asignarMarcoAProceso(ID, tablaINV, tamanioTabla) == -1) { //Se asigna marco y se checkea error
			return -1; //No hay marcoooosssss
		}
		pagina = 0; //Si asignarMarcoAProceso sale bien, la pagina termina siendo 0 en este caso
	}

	int marco = TI_buscarMarco(ID, pagina, tablaINV, tamanioTabla);

	if (TI_marcoEstaLleno(marco, tablaInfo, tamanioTabla, tamPag, tamLinea)) {
		if (TI_asignarMarcoAProceso(ID, tablaINV, tamanioTabla) == -1) {
			return -1; //No hay marcoooosssss
		}
		pagina++; //Al asignarle otro marco al proceso, la nueva pagina va a ser la anterior+1
		marco = TI_buscarMarco(ID, pagina, tablaINV, tamanioTabla);
	}

	return marco;
}

int TI_agregarLineaPrimera(int ID, char* linea, t_list* tablaInfo,
		t_list* tablaINV, int tamPag, int tamanioTabla, int tamLinea,
		t_log* logger) { //Dado un proceso y una linea, agrega la linea en un marco vacio

	//Esta funcion puede contener leftovers

	int marco = TI_devolverMarcoVacioUsableParaProceso(ID, tablaINV, tablaInfo,
			tamanioTabla, tamPag, tamLinea); //Aca se considera si el marco esta lleno

	if (marco == -1) { //Si no quedan disponibles
		return -1;
	}

	memcpy(storage + (marco * tamPag), linea, tamLinea);

	for (int i = 0; i < tamanioTabla; i++) { //Se actualiza lineasOcupadas
		t_TI_InfoAux* elemento = list_get(tablaInfo, i);

		if (elemento->marco == marco) {
			elemento->lineasOcupadas++;
			log_info(logger, "Se ha agregado una linea en la memoria");
			return marco;
		}
	}
	return -1; //el error maximo
}

int TI_agregarLinea(int ID, char* linea, t_list* tablaInfo, t_list* tablaINV,
		int tamPag, int tamanioTabla, int tamLinea, t_log* logger, int marco) { //Agrega una linea para un proceso en marco

	int offset = TI_devolverOffsetVacioDeMarco(marco, tablaInfo, tamanioTabla);

	memcpy(storage + (marco * tamPag) + (offset * tamLinea), linea, tamLinea);

	for (int i = 0; i < tamanioTabla; i++) { //Se actualiza lineasOcupadas
		t_TI_InfoAux* elemento = list_get(tablaInfo, i);

		if (elemento->marco == marco) {
			elemento->lineasOcupadas++;
			log_info(logger, "Se ha agregado una linea en la memoria");
			return 1;
		}
	}
	return -1; //el error maximo
}

int TI_devolverLinea(int marco, int offset, int tamanioTabla, int tamanioLinea,
		int tamanioPag, t_list* tablaInfo, t_list* tablaINV, t_log* logger,
		char** linea) {

	int marcoReal = marco;
	int offsetReal = offset;

	if (offset >= (tamanioPag / tamanioLinea)) //Si el offset es "invalido"
			{
		int paginasSobrantes = offset / (tamanioPag / tamanioLinea);
		t_TablaINV* elemento = list_get(tablaINV, marco);
		int paginaReal = elemento->pagina;

		for (int i = 0; i < paginasSobrantes; i++) {
			paginaReal = TI_proximaPagina(elemento->PID, paginaReal);
		}

		int ID = elemento->PID;

		marcoReal = TI_buscarMarco(ID, paginaReal, tablaINV, tamanioTabla);
		offsetReal = offset % (tamanioPag / tamanioLinea);
	}

	if (offsetReal
			>= TI_devolverOffsetVacioDeMarco(marcoReal, tablaInfo,
					tamanioTabla)) //Si realmente es invalido
					{
		return 0;
	}

	memcpy(*linea,
			storage + (marcoReal * tamanioPag) + (offsetReal * tamanioLinea),
			tamanioLinea);
	return 1;

}

int TI_modificarLinea(int marco, int offset, char* linea, int tamanioTabla,
		int tamanioLinea, int tamanioPag, t_list*tablaInfo, t_list*tablaINV,
		t_log* logger) {
	//Cambia la linea en la posicion marco+offset

	int marcoReal = marco;
	int offsetReal = offset;

	if (offset >= (tamanioPag / tamanioLinea)) //Si el offset es "invalido"
			{
		int paginasSobrantes = offset / (tamanioPag / tamanioLinea);
		t_TablaINV* elemento = list_get(tablaINV, marco);
		int paginaReal = elemento->pagina;

		for (int i = 0; i < paginasSobrantes; i++) {
			paginaReal = TI_proximaPagina(elemento->PID, paginaReal);
		}

		int ID = elemento->PID;

		marcoReal = TI_buscarMarco(ID, paginaReal, tablaINV, tamanioTabla);
		offsetReal = offset % (tamanioPag / tamanioLinea);
	}

	if (offsetReal
			>= TI_devolverOffsetVacioDeMarco(marcoReal, tablaInfo,
					tamanioTabla)) //Si realmente es invalido
					{
		return 0;
	}

	memcpy(storage + (marcoReal * tamanioPag) + (offsetReal * tamanioLinea),
			linea, tamanioLinea);

	return 1;

}

void TI_removerPagina(int ID, int pagina) {

	//Se asume que nunca va a haber un marco vacio entre marcos ocupados
	//Se asume que "pagina" existe

	int primerMarco = TI_buscarMarco(ID, pagina, tablaInvertida, tamanioTabla);
	int marcosParaMover = 0;

	for (int i = primerMarco + 1; i < tamanioTabla; i++) {
		t_TablaINV* elementoINV = list_get(tablaInvertida, i);

		if (elementoINV->pagina != -1) {
			marcosParaMover++;
		}

	}

	if (marcosParaMover == 0) //Si es el ultimo marco ocupado
			{
		//Se desasigna el marco

		t_TablaINV* elementoINVE = list_get(tablaInvertida, primerMarco);
		elementoINVE->PID = -1;
		elementoINVE->pagina = -1;

		t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar, primerMarco);
		elementoInfo->lineasOcupadas = 0;
		elementoInfo->terminaUnArchivo = 0;

		return;
	}

	for (int d = 0; d < marcosParaMover; d++) {
		t_TablaINV* elementoINVPrimero = list_get(tablaInvertida,
				primerMarco + d);
		t_TablaINV* elementoINVProximo = list_get(tablaInvertida,
				primerMarco + d + 1);

		t_TI_InfoAux* elementoInfoPrimero = list_get(TI_tablaInfoAuxilar,
				primerMarco + d);
		t_TI_InfoAux* elementoInfoProximo = list_get(TI_tablaInfoAuxilar,
				primerMarco + d + 1);

		elementoINVPrimero->PID = elementoINVProximo->PID;
		elementoINVPrimero->pagina = elementoINVProximo->pagina;

		elementoInfoPrimero->lineasOcupadas =
				elementoInfoProximo->lineasOcupadas;
		elementoInfoPrimero->terminaUnArchivo =
				elementoInfoProximo->terminaUnArchivo;

		elementoINVProximo->PID = -1;
		elementoINVProximo->pagina = -1;
		elementoInfoProximo->lineasOcupadas = 0;
		elementoInfoProximo->terminaUnArchivo = 0;

		//if (d != marcosParaMover - 1) //Para evitar un posible seg fault
		//		{
			memcpy(storage + (primerMarco + d) * tam_max_pag,
					storage + (primerMarco + d + 1) * tam_max_pag, tam_max_pag);
		//}

	}

}

int TI_devolverMarcoVacioUsableParaProceso(int ID, t_list* tablaINV,
		t_list* tablaInfo, int tamanioTabla, int tamPag, int tamLinea) { //Devuelve un marco donde el proceso "ID" pueda escribir una nueva linea

	int pagina = TI_buscarPaginaMasAlta(ID, tablaINV, tamanioTabla);

	if (pagina == -1) { //Osea si no tenia ninguna pagina en memoria
		int marcoAux = TI_asignarMarcoAProceso(ID, tablaINV, tamanioTabla);
		if (marcoAux == -1) { //Se asigna marco y se checkea error
			return -1; //No hay marcoooosssss
		}
		//Si asignarMarcoAProceso sale bien, la pagina termina siendo 0 en este caso

		t_TablaINV* elementoINV = list_get(tablaInvertida, marcoAux);
		pagina = elementoINV->pagina;

		int marco = TI_buscarMarco(ID, pagina, tablaINV, tamanioTabla);
		return marco; //Como el marco es nuevo, esta vacio, no es necesario checkear
	}

	int marco = TI_buscarMarco(ID, pagina, tablaINV, tamanioTabla);

	if (TI_marcoEstaVacio(marco, tablaInfo, tamanioTabla, tamPag, tamLinea)) { //Si el marco de la pagina mas alta esta vacio se devuelve el marco
		return marco;
	}

	//Si no esta vacia la ultima pagina

	return TI_asignarMarcoAProceso(ID, tablaINV, tamanioTabla); //Se devuelve -1 si no hay espacio o el marco si es que hay
}

int TI_proximaPagina(int ID, int pagina) {
	int proximaPagina = pagina;

	while (1) {
		proximaPagina++;

		if (TI_buscarMarco(ID, proximaPagina, tablaInvertida, tamanioTabla)
				!= -1) {
			return proximaPagina;
		}

		if (proximaPagina == 10000) //Antes era INT_MAX
				{
			return -1;
		}

	}
}

void TI_enviarPaginaAlDiego(int ID, int pagina, int diego) {
	int marco = TI_buscarMarco(ID, pagina, tablaInvertida, tamanioTabla);

	t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar, marco);
	int tamLin;

	for (int i = 0; i < elementoInfo->lineasOcupadas; i++) {

		char* linea = malloc(tam_max_linea);

		TI_devolverLinea(marco, i, tamanioTabla, tam_max_linea, tam_max_pag,
				TI_tablaInfoAuxilar, tablaInvertida, loggerFM9, &linea);

		string_trim_right(&linea);
		tamLin = strlen(linea) + 1;
		log_info(loggerFM9, "El tamanio de la linea enviada es: %d", tamLin);
		log_info(loggerFM9, "la linea es: %s", linea);
		send(diego, &tamLin, 4, 0);
		send(diego, linea, tamLin, 0);
		free(linea);
	}
}

int TI_lineasEnMemoriaDeArchivo(int marcoInicial)
{
	int cantidadLineas = 0;

	for (int i= marcoInicial ; i<tamanioTabla ; i++)
	{
		t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar,i);

		cantidadLineas = cantidadLineas + elementoInfo->lineasOcupadas;

		if (elementoInfo->terminaUnArchivo)
		{
			break;
		}

	}

	return cantidadLineas;
}

void TI_mostrarEstadisticas()
{
	printf("\n\n_________________________________________________________\n");
	printf("Estado de la memoria:\n\n");


	for (int i = 0; i<tamanioTabla ; i++)
	{
		t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar,i);

		if (elementoInfo->lineasOcupadas == 0 && i == tamanioTabla-1)
		{
			printf("El marco %i esta vacio\n",i);
			break;
		}

		if (elementoInfo->lineasOcupadas == 0)
		{
			printf("Los marcos %i - %i estan vacios\n",i,tamanioTabla-1);
			break;
		}



		int porcentaje = (elementoInfo->lineasOcupadas*100)/(tam_max_pag/tam_max_linea);
		printf("Marco %i: ",i);
		printf("%i%% lleno\n",porcentaje);
	}
	printf("\n_________________________________________________________\n\n");
}

int TI_lineasLibresEnMemoria()
{
	int lineas = 0;
	int lineasPorPagina = tam_max_pag/tam_max_linea;

	for (int i = 0 ; i<tamanioTabla ; i++)
	{
		t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar,i);

		lineas = lineas + (lineasPorPagina - elementoInfo->lineasOcupadas);
	}

	return lineas;
}


void liberar_tablaInvertida(t_list*tablaINV, t_list*tablaINFO, int tamanioTabla) {
	for (int i = tamanioTabla - 1; i != -1; i--) {
		t_TablaINV* elementoINV = list_get(tablaINV, i);
		t_TablaINV* elementoINFO = list_get(tablaINFO, i);

		free(elementoINV);
		free(elementoINFO);

	}
}



