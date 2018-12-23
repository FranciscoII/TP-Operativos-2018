#include "funcionesFM9.h"
extern t_config* config;
extern t_list* tablaSegmentos;
extern t_list* tablaInvertida;
extern t_list* TI_tablaInfoAuxilar;
extern t_log* loggerFM9;
extern int tam_max_linea;
extern int tamanioTabla;
extern int tam_max_pag;
extern char* storage;
extern int tam_max_pag;
int proximoID = 0;
t_list* tablaPagAuxiliar;
extern sem_t mutexTabla;

int devolverModo(t_config*config) {
	char* modoChar = config_get_string_value(config, "MODO");

	if (!strcmp(modoChar, "SEG")) {
		return SEG;
	}
	if (!strcmp(modoChar, "TPI")) {
		return PAG;
	}

	if (!strcmp(modoChar, "SEGPAG")) {
		return SEGPAG;
	}
	free(modoChar);
	return -1;

}

int buscarLineaPedida(int pid, int SoP, int offset, int procesador) {
	int bas;
	int lim;
	char* lin = malloc(tam_max_linea);
	char* error = "ERROR";
	int tamLin = 0;
	switch (devolverModo(config)) {
	case SEG:
		bas = encontrarBaseSegmento(SoP);
		lim = encontrarLimiteSegmento(SoP);
		//log_info(loggerFM9,"La base y el segmento tan piolas");
		printf("La base y el segmento son %d %d\n", bas, lim);
		if (bas > -1 && offset < lim) {
			memcpy(lin, storage + (bas + offset) * tam_max_linea,
					tam_max_linea);
			tamLin = strlen(lin) + 1;
			send(procesador, &tamLin, 4, 0);
			send(procesador, lin, tamLin, 0); //le mando el tam max y el cpu despues lo recorta.
			log_info(loggerFM9, "Enviado");
			free(lin);
			return 1;
		}
		break;
	case PAG:
		;
		int pagina = SoP;

		if (TI_buscarMarco(pid, pagina, tablaInvertida, tamanioTabla) == -1) {
			pagina = TI_proximaPagina(pid, pagina);
		}

		int marco = TI_buscarMarco(pid, pagina, tablaInvertida, tamanioTabla);

		int respuesta = TI_devolverLinea(marco, offset, tamanioTabla,
				tam_max_linea, tam_max_pag, TI_tablaInfoAuxilar, tablaInvertida,
				loggerFM9, &lin);

		if (respuesta == 0) {
			log_error(loggerFM9, "La linea no pudo ser encontrada");
			free(lin);
			tamLin = strlen(error) + 1;
			send(procesador, &tamLin, 4, 0);
			send(procesador, error, tamLin, 0);
			break;

		}

		tamLin = strlen(lin) + 1;
		send(procesador, &tamLin, 4, 0);
		send(procesador, lin, tamLin, 0);

		log_info(loggerFM9, "Enviado");
		free(lin); //Si algo no funciona en paginacion, ver este free primero!
		return 1;

	case SEGPAG:
		bas = encontrarMarco(SoP, offset);
		printf("el marco es %d\n", bas);
		int tamLineasPag = tam_max_pag / tam_max_linea;
		offset = offset % tamLineasPag;
		if (bas >= 0) {
			memcpy(lin, storage + (bas*tamLineasPag + offset) * tam_max_linea,
					tam_max_linea);
			tamLin = strlen(lin) + 1;
			send(procesador, &tamLin, 4, 0);
			send(procesador, lin, tamLin, 0); //le mando el tam max y el cpu despues lo recorta.
			log_info(loggerFM9, "Enviado");
			free(lin);
			return 1;
		}
		break;
	default:
		;
	}
	log_info(loggerFM9,
			"NO se encontro la linea pedida, enviando mensaje de error");
	tamLin = strlen(error) + 1;
	send(procesador, &tamLin, 4, 0);
	send(procesador, error, tamLin, 0);
	return 0;
}

int realizarAccion(void* paquete) {
//el paquete contiene 3 int
//1°Accion a realizar(MDJ A FM9) 2°NumLineas 3° PID
	int accion;
	int base;
	int numeroDeLineas;
	int pagina;
	int marco;
	int PID;
	int memoryPointer;
	int numeroMarcos;
	int nro;
	int tamLineasPag = tam_max_pag / tam_max_linea;
	memcpy(&PID, paquete + 8, 4);
	memcpy(&accion, paquete, 4);
	memcpy(&nro, paquete+4, 4);
	switch (accion) {
	case 1:		//recibir lineas y guardarlas
		memcpy(&numeroDeLineas, paquete + 4, 4);
		switch (devolverModo(config)) {
		case SEG:
			base = obtenerBaseParaSegDeNLineas(numeroDeLineas);
			log_info(loggerFM9, "Buscando espacio dentro del area");
			return base;
			break;
		case PAG:

			if( TI_lineasLibresEnMemoria() < nro)
			{
				return -1;
			}

			sem_wait(&mutexTabla);
			marco = TI_devolverMarcoVacioUsableParaProceso(PID, tablaInvertida,
					TI_tablaInfoAuxilar, tamanioTabla, tam_max_pag,
					tam_max_linea);
			sem_post(&mutexTabla);

			if (marco == -1) {
				return -1;
			}

			t_TablaINV* elemento = list_get(tablaInvertida, marco);
			pagina = elemento->pagina;
			return pagina;
			break;
		case SEGPAG:
			numeroMarcos = numeroDeLineas / tamLineasPag;
			if(numeroDeLineas % tamLineasPag)
				numeroMarcos++;
			if (!hayBloquesDisponibles(numeroMarcos)) //verifica si hay suficientes marcos
				return -1;
			else {
				log_info(loggerFM9, "Encontre espacio dentro del area");
				base = obtenerPrimerMarcoLibre();
				return base;
			}
			break;
		}
		//fin switch(devolverModo)
		break;
	case 2: //interpretacion de la panela (ACCION, SEGMENTO,PID)
		//Nota: Dice segmento pero es lo mismo que manda el CPU pero sin el offset
		memcpy(&memoryPointer, paquete + 4, 4);
		if (estaEnMemoria(memoryPointer, PID))
			return FLUSH;
		break;
	case 3: //liberar el escriptorio de memoria
		memcpy(&memoryPointer, paquete + 4, 4);
		liberarMemoria(PID, memoryPointer);
		return -3;
	default:
		;
	}
	return -1; //ERROR
}

int estaEnMemoria(int memPoint, int PID) {
	int aux;
	switch (devolverModo(config)) {
	case SEG:
		aux = encontrarBaseSegmento(memPoint); //Busca en las tablas a ver si hay algun seg asociado.
		break;
	case PAG:
		aux = TI_buscarMarco(PID, memPoint, tablaInvertida, tamanioTabla);
		break;
	case SEGPAG:
		aux = encontrarMarco(memPoint, 0);
		break;
	}
	if (aux >= 0)
		return 1;		//Se encuentra en memoria
	else
		return 0;		//No esta en memoria
}

void recibirYGuardarLineas(int socketDiego, int nro, int base, int ID) {
	char*bufferREC = malloc(tam_max_linea);
	int limitePag = tam_max_pag / tam_max_linea;
	printf("Cada pagina tiene esta cantidad de Lineas: %d", limitePag);
	int pag = 0;
	int ayu;
	tablaPagAuxiliar = list_create();
	switch (devolverModo(config)) {
	case SEG:
		for (int k = 0; k < nro; k++) {
			int tamLinea = 0;
			recv(socketDiego, &tamLinea, 4, MSG_WAITALL);
			log_info(loggerFM9, "el tamanio de linea es %d", tamLinea);
			recv(socketDiego, bufferREC, tamLinea, MSG_WAITALL);
			log_info(loggerFM9, "la linea es %s", bufferREC);
			memcpy(storage + (base + k) * tam_max_linea, bufferREC,
					tam_max_linea);
			setear(base + k);
			log_info(loggerFM9, "lol recibi una linea bien");
		}
		free(tablaPagAuxiliar);		//Por el memory leak que genera la linea 408
		break;
	case PAG:
		;

		int error;
		int tamLinea = 0;
		int marco;
		recv(socketDiego, &tamLinea, 4, MSG_WAITALL);
		log_info(loggerFM9, "el tamanio de linea es %d", tamLinea);
		recv(socketDiego, bufferREC, tamLinea, MSG_WAITALL);
		log_info(loggerFM9, "la linea es %s", bufferREC);
		sem_wait(&mutexTabla);
		error = TI_agregarLineaPrimera(ID, bufferREC, TI_tablaInfoAuxilar,
				tablaInvertida, tam_max_pag, tamanioTabla, tam_max_linea,
				loggerFM9);
		sem_post(&mutexTabla);
		marco = error; //Si error no es un error, es el marco

		if (error == -1) {

			log_error(loggerFM9,
					"No queda espacio disponible para agregar una linea");
			free(tablaPagAuxiliar);
			return;

		}
		if (nro == 1) //Si solo se esta agregando una linea
				{
			t_TI_InfoAux* elemento = list_get(TI_tablaInfoAuxilar, marco);
			elemento->terminaUnArchivo = 1;
		}

		for (int k = 1; k < nro; k++) {
			tamLinea = 0;
			recv(socketDiego, &tamLinea, 4, MSG_WAITALL);
			log_info(loggerFM9, "el tamanio de linea es %d", tamLinea);
			recv(socketDiego, bufferREC, tamLinea, MSG_WAITALL);
			log_info(loggerFM9, "la linea es %s", bufferREC);

			if (TI_marcoEstaLleno(marco, TI_tablaInfoAuxilar, tamanioTabla,
					tam_max_pag, tam_max_linea)) {
				sem_wait(&mutexTabla);
				error = TI_agregarLineaPrimera(ID, bufferREC,
						TI_tablaInfoAuxilar, tablaInvertida, tam_max_pag,
						tamanioTabla, tam_max_linea, loggerFM9);
				sem_post(&mutexTabla);
				marco = error;

			} else {
				sem_wait(&mutexTabla);
				error = TI_agregarLinea(ID, bufferREC, TI_tablaInfoAuxilar,
						tablaInvertida, tam_max_pag, tamanioTabla,
						tam_max_linea, loggerFM9, marco);
				sem_post(&mutexTabla);
			}

			if (error == -1) {

				log_error(loggerFM9,
						"No queda espacio disponible para agregar una linea");
				free(tablaPagAuxiliar); //Por el memory leak que genera la linea 408
				return;

			}

			if (k == nro - 1) //Si es la ultima linea que se agrega
					{
				t_TI_InfoAux* elemento = list_get(TI_tablaInfoAuxilar, marco);
				elemento->terminaUnArchivo = 1;
			}

		}
		free(tablaPagAuxiliar); //Por el memory leak que genera la linea 408
		break;

	case SEGPAG: //aca la base es el nro de marco por lo que a ese numero hay que mult por tamMaxLinea
		//ayu = tam_max_pag/tam_max_linea;//CANTIDAD LINEAS POR MARCO
		for (int k = 0; k < nro; k++) {
			if (k < limitePag) {
				int tamLinea = 0;
				recv(socketDiego, &tamLinea, 4, MSG_WAITALL);
				log_info(loggerFM9, "el tamanio de linea es %d", tamLinea);
				recv(socketDiego, bufferREC, tamLinea, MSG_WAITALL);
				log_info(loggerFM9, "la linea es %s", bufferREC);
				memcpy(storage + (base*limitePag + k) * tam_max_linea, bufferREC,
						tam_max_linea);
				log_info(loggerFM9, "lol recibi una linea bien");
				ayu = k;
			} else { //esta escribiendo en el siguiente marco
				agregarPagALista(base, pag);
				setear(base);
				base = obtenerPrimerMarcoLibre();
				pag++;
				k = -1; //por el for()
				ayu=-1;
				nro = nro - limitePag; //Chicos esto lo hice a las 3AM... Atte. Francisco
			}
		}
		if (ayu > -1) {//si queda un marco incompleto lo agrega a la lista
			printf("Agregada la pagina incompleta\n");
			agregarPagALista(base, pag);
			setear(base);
		}
		memcpy(&base, &proximoID, 4);
		break;
	}
	free(bufferREC);
}
int enviarMemoriaADiego(int diego, void* paquete) { //Paquete:(flush,IDMEMORIA,PID)
	int base;
	int limite;
	int idMem;
	int pid;
	int tamLin;
	int aux = 0;
	memcpy(&idMem, paquete + 4, 4);
	memcpy(&pid, paquete + 8, 4);
	switch (devolverModo(config)) {
	case SEG:
		base = encontrarBaseSegmento(idMem);
		limite = encontrarLimiteSegmento(idMem);
		log_info(loggerFM9, "Empezando a enviar a Diego");
		log_info(loggerFM9, "Enviando cantidad Lineas %d", limite);
		send(diego, &limite, 4, 0);
		for (int k = 0; k < limite; k++) {
			char* linea = malloc(tam_max_linea);
			memcpy(linea, storage + (base + k) * tam_max_linea, tam_max_linea);

			string_trim_right(&linea);
			tamLin = strlen(linea) + 1;
			log_info(loggerFM9, "El tamanio de la linea enviada es: %d",
					tamLin);
			log_info(loggerFM9, "la linea es: %s", linea);
			//string_append(&linea,"\n");
			//sin el transferSize
			send(diego, &tamLin, 4, 0);
			send(diego, linea, tamLin, 0);
//				mandarLineaADiego(diego,linea);
			free(linea);
		}
		break;

	case PAG:
		;

		int paginasSobrantes = 0;
		int marco = TI_buscarMarco(pid, idMem, tablaInvertida, tamanioTabla);
		int cantidadLineas = TI_lineasEnMemoriaDeArchivo(marco);

		log_info(loggerFM9, "Empezando a enviar a Diego");
		log_info(loggerFM9, "Enviando cantidad Lineas %d", cantidadLineas);
		send(diego, &cantidadLineas, 4, 0);

		while (1) {
			t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar, marco);

			if (elementoInfo->terminaUnArchivo == 1) {
				break;
			}

			paginasSobrantes++;

			t_TablaINV* elementoINV = list_get(tablaInvertida, marco);

			int pagina = elementoINV->pagina;

			marco = TI_buscarMarco(pid, TI_proximaPagina(pid, pagina),
					tablaInvertida, tamanioTabla);

			if (marco == -1) {
				log_error(loggerFM9, "Algo salio MUY mal");
				break;
			}

		}

		marco = TI_buscarMarco(pid, idMem, tablaInvertida, tamanioTabla);

		t_TablaINV* elementoINV = list_get(tablaInvertida, marco);
		int pagina = elementoINV->pagina;

		for (int i = 0; i <= paginasSobrantes; i++) {

			TI_enviarPaginaAlDiego(pid, pagina, diego);

			if (i != paginasSobrantes) //Para evitar un seg fault
					{
				pagina = TI_proximaPagina(pid, pagina);
			}

		}

		break;
	case SEGPAG:
		limite = encontrarLimite(pid, idMem);
		base = encontrarMarco(idMem, 0);
		int tamLineasPag = tam_max_pag / tam_max_linea;
		log_info(loggerFM9, "Empezando a enviar a Diego");
		log_info(loggerFM9, "enviando cantidad Lineas %d", limite);
		send(diego, &limite, 4, 0);
		for (int k = 0; k < limite; k++) {
			char* linea = malloc(tam_max_linea);
			if (k < tamLineasPag) {
				memcpy(linea, storage + (base*tamLineasPag + k) * tam_max_linea,
						tam_max_linea);
				string_trim_right(&linea);
				tamLin = strlen(linea) + 1;
				log_info(loggerFM9, "El tamanio de la linea enviada es: %d",
						tamLin);
				log_info(loggerFM9, "Le envio esta linea al Diego: %s", linea);
				send(diego, &tamLin, 4, 0);
				send(diego, linea, tamLin, 0);
			} else {
				aux += k;
				base = encontrarMarco(idMem, aux);
				if (base < 0)
					break;
				k = -1;
				limite -= tamLineasPag;
			}
			free(linea);
		}
		break;
	}
	log_info(loggerFM9, "Enviadas las lineas al Diegote");
	return 0;
}

void actualizarListas(int base, void* paquete) {
	int numeroDeLineas;
	//t_segmentoSEGPAG* nuevoSegmentoPag = malloc(sizeof(t_segmentoSEGPAG));
	int Proceso;
	memcpy(&Proceso, paquete + 8, sizeof(int));
	memcpy(&numeroDeLineas, paquete + 4, sizeof(int));
	t_segmentacion* nuevoSegmento;
	switch (devolverModo(config)) {
	case SEG:
		nuevoSegmento = malloc(sizeof(t_segmentacion));
		nuevoSegmento->base = base;
		nuevoSegmento->limite = numeroDeLineas;
		nuevoSegmento->id = proximoID;
		nuevoSegmento->PID = Proceso;
		int nromagico = list_add(tablaSegmentos, nuevoSegmento);
		printf("El nuevo segmento tiene ID: %d Limite. %d Base: %d\n",
				proximoID, numeroDeLineas, base);
		proximoID++;
		break;
	case PAG:	//Se hace dentro de las funciones
		break;
	case SEGPAG:
		memcpy(&numeroDeLineas, paquete + 4, sizeof(int));//le agrego esto solo para que funcione lol
		t_segmentoSEGPAG* nuevoSegmentoPag = malloc(sizeof(t_segmentoSEGPAG));
		nuevoSegmentoPag->id = proximoID;
		nuevoSegmentoPag->PID = Proceso;
		nuevoSegmentoPag->limite = numeroDeLineas;
		nuevoSegmentoPag->tabla = list_create();
		list_add_all(nuevoSegmentoPag->tabla, tablaPagAuxiliar);
		list_add(tablaSegmentos, nuevoSegmentoPag);
		//ahora debo liberar la tabla auxiliar
//		log_info(loggerFM9,"Actualizada lista de Pag");
		list_destroy(tablaPagAuxiliar);
		informarLista(Proceso);
		proximoID++;
		break;
	}
}

void mandarLineaADiego(int diego, char*linea) {	//NO SE USA ESTA FUNCION EN EL FUTURO CAPAZ LA BORRE
	void*paquete = malloc(strlen(linea) + 1 + 4);
	//int transferSize=config_get_int_value(config,"TRANSFER_SIZE");
	int transferSize = 16;	//HARDCODEADO XD
	int tamanioPaquete = strlen(linea) + 1 + 4;
	int tamanioLinea = strlen(linea) + 1;
	memcpy(paquete, &tamanioLinea, 4);
	memcpy(paquete + 4, (void*) linea, tamanioLinea);

	int tamanioAEnviar;
	if (transferSize > tamanioPaquete) {
		tamanioAEnviar = tamanioPaquete;
	} else {
		tamanioAEnviar = transferSize;
	}

	int ciclosCompletos = tamanioPaquete / transferSize;
	int cicloIncompleto = tamanioPaquete % transferSize;
	int contador = 0;
	for (int i = 0; i < ciclosCompletos; i++) {
		send(diego, paquete + i * transferSize, tamanioAEnviar, 0);
		contador++;
	}
	send(diego, paquete + contador * transferSize, cicloIncompleto, 0);
	free(paquete);
}
void informarLista(int PID) {
	t_segmentacion*segBusc;	// = malloc(sizeof(t_segmentacion));
	t_segmentoSEGPAG* segBuscado;	// = malloc(sizeof(t_segmentoSEGPAG));
	t_paginaSEGPAG* pagBusc;	// = malloc(sizeof(t_paginaSEGPAG));
	t_TablaINV* elementoBuscado;	// = malloc(sizeof(t_TablaINV));

	switch (devolverModo(config)) {
	case SEG:
		for (int i = 0; i < list_size(tablaSegmentos); i++) {
			segBusc = list_get(tablaSegmentos, i);

			if (segBusc->PID == PID) {
				printf("PID: %d\n", segBusc->PID);
				printf("ID Segmento: %d\n", segBusc->id);
				printf("Base: %d\n", segBusc->base);
				printf("Limite: %d\n", segBusc->limite);
				printf("LINEAS:\n");
				printearLineas(segBusc->PID, segBusc->id);
			}
		}
		break;
	case PAG:
		;
		int pagina = 0;

		if (TI_buscarMarco(PID, pagina, tablaInvertida, tamanioTabla) == -1) {
			pagina = TI_proximaPagina(PID, pagina);
		}

		while (1) {
			int marco = TI_buscarMarco(PID, pagina, tablaInvertida,
					tamanioTabla);

			if (marco == -1) {
				break;
			}

			elementoBuscado = list_get(tablaInvertida, marco);

			printf("\n\nPID: %d\n", PID);
			printf("Numero de marco: %d\n", elementoBuscado->indice);
			printf("Numero de pagina: %d\n", elementoBuscado->pagina);
			printf("LINEAS:\n");

			printearLineas(PID, elementoBuscado->indice);

			pagina = TI_proximaPagina(PID, pagina);

		}
		printf("\n_________________________________________________________\n\n");

		break;
	case SEGPAG:
		for (int i = 0; i < list_size(tablaSegmentos); i++) {
			segBuscado = list_get(tablaSegmentos, i);

			if (segBuscado->PID == PID) {

				printf("PID: %d\n", segBuscado->PID);
				printf("ID Segmento: %d\n", segBuscado->id);
//				for (int j = 0; j < list_size(segBuscado->tabla); j++) {
//					pagBusc = list_get(segBuscado->tabla, j);
//					printf("NroPag: %d \n", pagBusc->nroPag);
//					printf("Marco que la almacena: %d \n", pagBusc->marco);
//					printf("LINEAS: \n");
				int pidB = segBuscado->PID;
				int idB = segBuscado->id;
				printearLineas(pidB, idB);
				//	}

			}
		}
		break;
	}
//	free(segBusc);
//	free(segBuscado);
//	free(pagBusc);
}
void printearLineas(int pid, int id) {
	int bas;
	int lim;
	int aux = 0;
	char* linea = malloc(tam_max_linea);
	switch (devolverModo(config)) {
	case SEG:
		bas = encontrarBaseSegmento(id);
		lim = encontrarLimiteSegmento(id);
		for (int k = 0; k < lim; k++) {
			memcpy(linea, storage + (bas + k) * tam_max_linea, tam_max_linea);
			printf("%s", linea);
		}

		break;
	case PAG: //En PAG "id" es el marco

		for (int d = 0; d < tam_max_pag / tam_max_linea; d++) {
			int respuesta = TI_devolverLinea(id, d, tamanioTabla, tam_max_linea,
					tam_max_pag, TI_tablaInfoAuxilar, tablaInvertida, loggerFM9,
					&linea);

			if (respuesta == 0) //Si no hay ninguna otra linea
					{
				break;
			}

			printf("%s", linea);
			free(linea);
			linea = malloc(tam_max_linea);

		}

		break;
	case SEGPAG:
		lim = encontrarLimite(pid, id);
		//printf("LIMITE ES: %d",lim);
		bas = encontrarMarco(id, 0);
		int tamLineasPag = tam_max_pag / tam_max_linea;
		for (int k = 0; k < lim; k++) {
			if (k < tamLineasPag) {
				memcpy(linea, storage + (bas*tamLineasPag + k) * tam_max_linea,
						tam_max_linea);
				printf("%s", linea);
			} else {
				aux += k;
				bas = encontrarMarco(id, aux);
				if (bas < 0)
					break;
				k = -1;
				lim -= tamLineasPag;
			}
		}
		break;
	}

	free(linea);
}

void cambiarPosicion(int pid, int SoP, int offset, char* datos, int cpuPiola) {
	int bas;
	int lim;
	int rtaCPU = 9;
	char* salto = "\n";
	switch (devolverModo(config)) {

	case SEG:
		bas = encontrarBaseSegmento(SoP);
		lim = encontrarLimiteSegmento(SoP);
		printf("La base y el segmento son %d %d\n", bas, lim);
		if (bas > -1 && offset < lim && strlen(datos) + 1 < tam_max_linea) {
			string_append(&datos, salto);
			memcpy(storage + (bas + offset) * tam_max_linea, datos,
					tam_max_linea);
			//agrego "\n"

		} else{
			log_error(loggerFM9, "Error: 20002");
			if(strlen(datos) + 1 < tam_max_linea)	log_error(loggerFM9, "Error: 20003");
				rtaCPU=-1;
				}
		break;
	case PAG:
		;
		int marco = TI_buscarMarco(pid, SoP, tablaInvertida, tamanioTabla);
		//string_append(&datos, salto);
		int resultado = TI_modificarLinea(marco, offset, datos, tamanioTabla,
				tam_max_linea, tam_max_pag, TI_tablaInfoAuxilar, tablaInvertida,
				loggerFM9);

		if (resultado == 0) {
			log_error(loggerFM9, "Escritura invalida");
			rtaCPU=-1;
		}

		break;
	case SEGPAG:
		lim = encontrarLimite(pid, SoP);
		bas = encontrarMarco(SoP, offset);
		int tamLineasPag = tam_max_pag / tam_max_linea;
		offset = offset % tamLineasPag;
		printf("La base y el segmento son %d %d\n", bas, lim);
		if (bas > -1 && offset < lim && strlen(datos) + 1 < tam_max_linea) {
			string_append(&datos, salto);
			memcpy(storage + (bas*tamLineasPag + offset) * tam_max_linea, datos,
					tam_max_linea);
		} else{
			log_error(loggerFM9, "Escritura invalida");
			rtaCPU=-1;
		}
		break;

	}
	send(cpuPiola,&rtaCPU,4,0);
	free(datos);
}

void liberarMemoria(int pid, int SoP) {
	int bas;
	int lim;
	int nro;
	int aux = 0;
	int index;
	int limitePag = tam_max_pag / tam_max_linea;
	char*blanco = " ";
	switch (devolverModo(config)) {
	case SEG:
		bas = encontrarBaseSegmento(SoP);
		lim = encontrarLimiteSegmento(SoP);
		log_info(loggerFM9, "Liberando segmento %d de %d lineas", bas, lim);
		if (bas > -1) {
			for (int i = 0; i < lim; i++) {
				desSetear(bas + i);
				memcpy(storage + (bas + i) * tam_max_linea, blanco,
						tam_max_linea);
			}
			index = liberarStructuraAdministrativa(pid, SoP);
			if (index == -1) {
				log_error(loggerFM9, "Ni idea que paso");
			} else {
				list_remove_and_destroy_element(tablaSegmentos, index,
						(void*) liberarSegmentos);
				//aca fallaba porque le estaba haciendo un doble free, en liberar estructuraAdministrativa
			}
		} else {
			log_info(loggerFM9,
					"no se encontraron los datos que querias liberar");
		}
		verArrayBits();
		informarLista(pid);
		break;
	case PAG:
		;

		int paginasSobrantes = 0;
		int marco = TI_buscarMarco(pid, SoP, tablaInvertida, tamanioTabla);

		while (1) {
			t_TI_InfoAux* elementoInfo = list_get(TI_tablaInfoAuxilar, marco);

			if (elementoInfo->terminaUnArchivo == 1) {
				break;
			}

			paginasSobrantes++;

			t_TablaINV* elementoINV = list_get(tablaInvertida, marco);

			int pagina = elementoINV->pagina;

			marco = TI_buscarMarco(pid, TI_proximaPagina(pid, pagina),
					tablaInvertida, tamanioTabla);

			if (marco == -1) {
				log_error(loggerFM9, "Algo salio MUY mal");
				break;
			}

		}

		marco = TI_buscarMarco(pid, SoP, tablaInvertida, tamanioTabla);

		t_TablaINV* elementoINV = list_get(tablaInvertida, marco);
		int pagina = elementoINV->pagina;

		for (int i = 0; i <= paginasSobrantes; i++) {

			TI_removerPagina(pid, pagina);

			if (i != paginasSobrantes) //Para evitar un seg fault
					{
				pagina = TI_proximaPagina(pid, pagina);
			}

		}

		break;
	case SEGPAG:
		bas = encontrarMarco(SoP, 0);
		nro = encontrarLimite(pid, SoP);
		for (int k = 0; k < nro; k++) {
			if (k < limitePag) {
				desSetear(bas + k);
				memcpy(storage + (bas*limitePag + k) * tam_max_linea, blanco,
										tam_max_linea);
			} else { //esta escribiendo en el siguiente marco
				aux += k;
				bas = encontrarMarco(SoP, aux); //si aux=3 por ej, nos devuelve el marco de la siguiente
				k = -1; //por el for()
				nro = nro - limitePag;
			} //Esto me genera muchas dudas habria que testearlo bien
		}
		index = liberarStructuraAdministrativa(pid, SoP);
		if (index == -1) {
			log_error(loggerFM9, "Ni idea que paso");
		} else {
			list_remove_and_destroy_element(tablaSegmentos, index,
					(void*) liberarSegmentosPag);
			//same pero en seg pag
		}
		verArrayBits();
		break;
	default:
		;
	}
}
int liberarStructuraAdministrativa(int PID, int SoP) {
	t_segmentacion*segBusc;
	t_segmentoSEGPAG* segBuscado;
	t_paginaSEGPAG* pagBusc;
	t_TablaINV* elementoBuscado;

	switch (devolverModo(config)) {
	case SEG:
		for (int i = 0; i < list_size(tablaSegmentos); i++) {
			segBusc = list_get(tablaSegmentos, i);
			if (segBusc->PID == PID && segBusc->id == SoP) {
				return i;
			}
		}
		break;
	case PAG:
		for (int i = 0; i < tamanioTabla; i++) {
			int marco = TI_buscarMarco(PID, i, tablaInvertida, tamanioTabla);

			if (marco == -1) {
				break;
			}

			elementoBuscado = list_get(tablaInvertida, marco);
		}

		break;
	case SEGPAG:
		for (int i = 0; i < list_size(tablaSegmentos); i++) {
			segBuscado = list_get(tablaSegmentos, i);

			if (segBuscado->PID == PID && segBuscado->id == SoP) {
				return i;
			}
		}
		break;
	}
	return -1;
}

