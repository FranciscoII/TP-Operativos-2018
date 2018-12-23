#include "funcionesFM9.h"
extern t_list* tablaSegmentos;
extern t_config* config;
extern t_log* loggerFM9;
t_list* tablaPagAuxiliar;
extern int tam_max_linea;
extern int tam_max_pag;
extern int tamanioMemoria;
int tambuffer;
int cantidadLineas;

void crear_Archivo_Bitmap() {
	cantidadLineas = tamanioMemoria/tam_max_linea;
	if(devolverModo(config)==SEGPAG){
		int lineasMarco =tam_max_pag/tam_max_linea;
		cantidadLineas=cantidadLineas/lineasMarco;
	}
	tambuffer = ((cantidadLineas%CHAR_BIT)==0)?(cantidadLineas/CHAR_BIT):((cantidadLineas/CHAR_BIT)+1);
	char a[tambuffer];
	t_bitarray *bitarray = bitarray_create(a, sizeof(a));
	for (int i = 0; i < cantidadLineas; i++)
		bitarray_clean_bit(bitarray, i);
	if ((cantidadLineas % CHAR_BIT) != 0)
		for (int j = cantidadLineas; j < tambuffer * CHAR_BIT; j++) {
			bitarray_set_bit(bitarray, j);
		}
	//
	FILE *fd =
			fopen(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FM9/Bitmap.bin",
					"wb");
	if (fd < 0 || fd == NULL) {
		printf("Errorrrr"); //seria un log
		exit(1);
	}
	fwrite(a, 1, tambuffer, fd);
	bitarray_destroy(bitarray);
	fclose(fd);
}

char* obtenerDatos() {
	char *buffer = malloc(tambuffer * sizeof(char));
	FILE *fd = fopen("/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders"
			"/FM9/Bitmap.bin", "rb");
	if (fd < 0 || fd == NULL) {
		printf("Errorrrr");
		exit(1);
	}
	fread(buffer, tambuffer, tambuffer, fd);
	fclose(fd);
	return buffer;
}

int hayBloquesDisponibles(int cantidad_bloques) {
	char *buffer = obtenerDatos();
	t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
	int cantidad = 0;
	for (int i = 0; (i < cantidadLineas) && (cantidad < cantidad_bloques); i++)
		if (!bitarray_test_bit(bitarray, i))
			cantidad++;
	bitarray_destroy(bitarray);
	free(buffer);
	return (cantidad >= cantidad_bloques);
}
int* obtenerIdBloquesDisponibles(int cantidad_bloques) {
	if (!hayBloquesDisponibles(cantidad_bloques))
		return NULL;
	else {
		char *buffer = obtenerDatos();
		t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
		int *a = malloc(cantidad_bloques * sizeof(int));
		int con = 0;
		for (int i = 0; (i < cantidadLineas) && (cantidad_bloques > 0); i++) {
			if (!bitarray_test_bit(bitarray, i)) {
				a[con] = i;
				cantidad_bloques--;
				con++;
			}
		}
		free(buffer);
		bitarray_destroy(bitarray);

		return a;
	}
}
int hayLineasContiguasDisponibles( cantidad_lineas) {
	char *buffer = obtenerDatos();
	t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
	int aux = cantidad_lineas;
	for (int i = 0; (i < cantidadLineas) && (cantidad_lineas > 0); i++) {
		if (!bitarray_test_bit(bitarray, i)) {
			cantidad_lineas--;
			for (int j = i + 1; (j < cantidadLineas) && (cantidad_lineas > 0);
					j++) {
				if (!bitarray_test_bit(bitarray, j)) {
					cantidad_lineas--;
				} else {
					cantidad_lineas = aux;
					break;
				}
			}
		}
	}
	bitarray_destroy(bitarray);
	free(buffer);
	if (cantidad_lineas > 0)
		return 0;
	else
		return 1; //TRUE

}
//BUSCA ESPACIO CONTIGUO EN MEMORIA PARA UN SEGMENTO
int obtenerBaseParaSegDeNLineas(int cantidad_Lineas) {
	int base = 0;
	//if(!hayBloquesDisponibles(cantidad_Lineas))
	if (!hayLineasContiguasDisponibles(cantidad_Lineas))
		return -1;
	else {
		char *buffer = obtenerDatos();
		t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
		int con = 0;
		int aux = cantidad_Lineas;
		for (int i = 0; (i < cantidadLineas) && (cantidad_Lineas > 0); i++) {
			if (!bitarray_test_bit(bitarray, i)) {
				base = i;
				cantidad_Lineas--;
				for (int j = i + 1;
						(j < cantidadLineas) && (cantidad_Lineas > 0); j++) {
					if (!bitarray_test_bit(bitarray, j)) {
						cantidad_Lineas--;
					} else {
						cantidad_Lineas = aux;
						break;
					}
				}
			}
		}
		bitarray_destroy(bitarray);
		free(buffer); //SI ROMPE VER ESTO
		return base;
	}
	return -1;
}

//SETEAR SE USA DENTRO DE OTRAS FUNCIONES DE ACA, NO COMPUREBA
//SI ESE BLOQUE YA ESTABA USADO
int setear(int idbloque) {
	char *buffer = obtenerDatos();
	t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
	bitarray_set_bit(bitarray, idbloque);
	FILE *fd =
			fopen(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FM9/Bitmap.bin",
					"wb");
	if (fd < 0 || fd == NULL) {
		bitarray_destroy(bitarray);
		free(buffer);
		return -1;
	}
	fwrite(buffer, 1, tambuffer, fd);
	free(buffer);
	bitarray_destroy(bitarray);
	fclose(fd);
	return 1;
}
int* reservarBloquesRequeridos(int cantidadBloques) {
	int *idBloquesDisponibles = obtenerIdBloquesDisponibles(cantidadBloques);
	if (idBloquesDisponibles == NULL)
		return NULL;
	for (int i = 0; i < cantidadBloques; i++) {
		if (setear(idBloquesDisponibles[i]) < 0) {
			for (int j = 0; j < i; j++)
				desSetear(idBloquesDisponibles[j]);
			return NULL;
		}

	}
	return idBloquesDisponibles;

}

int desSetear(int idbloque) {
	char *buffer = obtenerDatos();
	t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
	bitarray_clean_bit(bitarray, idbloque);
	FILE *fd =
			fopen(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/FM9/Bitmap.bin",
					"wb");
	if (fd < 0 || fd == NULL) {
		bitarray_destroy(bitarray);
		free(buffer);
		return -1;
	}
	fwrite(buffer, 1, tambuffer, fd);
	free(buffer);
	bitarray_destroy(bitarray);
	fclose(fd);
	return 1;
}

void verArrayBits() {
	char*buffer = obtenerDatos();
	t_bitarray *bitarray = bitarray_create(buffer, strlen(buffer));
	for (int i = 0; i < tambuffer * CHAR_BIT; i++) {
		if ((i % CHAR_BIT) == 0)
			printf("\n");
		printf("%d", bitarray_test_bit(bitarray, i));
	}
	printf("\n");
	free(buffer);
	bitarray_destroy(bitarray);
}

int encontrarBaseSegmento(int id) {
	t_segmentacion* segBusc;
	int tamTabla = list_size(tablaSegmentos);
	for (int i = 0; i < tamTabla; i++) {
		segBusc = list_get(tablaSegmentos, i);
		if (id == segBusc->id) {
			int basecita = segBusc->base;
			return (basecita);
		}
	}
	return -1;
}

int encontrarLimiteSegmento(int id) {
	t_segmentacion* segBusc;
	int limBusc;
	int tamTabla = list_size(tablaSegmentos);
	for (int i = 0; i < tamTabla; i++) {
		segBusc = list_get(tablaSegmentos, i);
		if (id == segBusc->id) {
			limBusc = segBusc->limite;
			return limBusc;
		}
	}
	return -1;
}

void liberar_tablaSegmentos(t_list*lista) {
	log_info(loggerFM9,"Liberando Tabla Segmentos");
	for (int i = list_size(lista) - 1; i >= 0; i--) {
		list_remove_and_destroy_element(lista, i, (void*) liberarSegmentos);
	}
}
void liberarSegmentos(t_segmentacion*paquete) {
	free(paquete);
}
