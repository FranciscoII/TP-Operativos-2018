#include "instrucciones.h"
#include<stdlib.h>
#include "consolaMDJ.h"

void retornarMensajeError(int error) {
	char* mistake;
	switch (error) {
	case AE:
		log_info(logger,"El archivo ya existe !\n");
		break;
	case EIA:
		log_info(logger,"Espacio insuficiente para el archivo !\n");
		break;
	case AIB:
		log_info(logger,"Se esta tratando de borrar un archivo inexistente !\n");
		break;
	case EIF:
		log_info(logger," Espacio insuficiente para guardar datos !\n");
		break;
	case AIF:
		log_info(logger," Se trata de guardar datos en un archivo inexistente !\n");
		break;
	case DIFICULTADESTECNICAS:
		log_info(logger,"Errores internos, estamos trabajando en ello.");
		break;
	}
}
devMDJ* retornarErrorYLoguearlo(int error, char*mensaje) {
	log_error(logger, mensaje);
	return retornarError(AIF);
}
devMDJ* validarArchivo(char *path) {
	char*patheando = crearCamino(path);
	FILE *fd = fopen(patheando, "rb");
	usleep(config_get_int_value(config, "RETARDO"));
	if (fd == NULL){
		free(patheando);
		return retornarError(AIF);
	}
	else {
		fclose(fd);
		free(patheando); //ACA CAMBIE
		return retornarExito();
	}
}
int verificarValidacion(char* path) {
	devMDJ* dev = validarArchivo(path); // @suppress("Type cannot be resolved")
	//printf("valido el archivo\n");
	if (dev->rta == TIPOERROR) { // @suppress("Field cannot be resolved")
		free(dev);
		return -1;
	}
	free(dev);
	return 1;
}
char* crearCamino(char*path) {
	char*patheando = string_new();
	string_append(&patheando, config_get_string_value(config,"PUNTO_MONTAJE"));
	if(string_starts_with(path,"/"))
		string_append(&patheando, "/Archivos");
	else
		string_append(&patheando, "/Archivos/");
	string_append(&patheando, path);
	return patheando;
}
devMDJ* retornarError(int error) {
	devMDJ *devolucion = malloc(sizeof(devMDJ));
	devolucion->rta = TIPOERROR;
	devolucion->error = error;
	devolucion->buffer = NULL;
	devolucion->tamBuffer = 0;
	retornarMensajeError(error);
	return devolucion;
}

devMDJ* retornarExito() {
	devMDJ *devolucion = malloc(sizeof(devMDJ));
	devolucion->rta = TODOOKEY;
	devolucion->error = 0;
	devolucion->buffer = NULL;
	devolucion->tamBuffer = 0;
	return devolucion;
}
devMDJ* crearArchivo(char *path, int nbytes) {
	log_info(logger, "CREO ARCHIVO %s", path);
	if (verificarValidacion(path) > 0) {
		printMDJ("ERROR: 50001 Archivo ya existente. \n");
		return retornarError(AE);
	}
	int canBloques = nbytes / TAM_BLOQUE;
	if ((nbytes % TAM_BLOQUE) != 0)
		canBloques++;
	if (cantidadDeBarras(path) > 0) {
		//printf("tiene subcarpetas tambien!\n");
		crearSubcarpetas(path);
	}
	char* patheando = crearCamino(path);
	int *buffer = reservarBloquesRequeridos(canBloques);
	if (!buffer){
		free(patheando);
		printMDJ("ERROR: 50002 Espacio insuficiente. \n");
		return retornarError(EIA);
	}
	crearFileMetadata(path, nbytes, buffer, canBloques);
	usleep(config_get_int_value(config, "RETARDO"));
	char inicio[nbytes];
	for (int i = 0; i < nbytes; i++)
		inicio[i] = '\n';
	devMDJ*dev = guardarDatos(path, 0, nbytes, inicio);
	free(dev);
	free(patheando);
	free(buffer);
	return retornarExito();
}

int inicializarBloques(char *path) {
	int *bloques = obtenerBloques(path);
	if (!bloques) {
		return -1;
	}
	for (int i = 0; i < cantidadBloques(path); i++) {
		char*patheando = caminoBloque(bloques[i]);
		FILE* f = fopen(patheando, "w");
		fclose(f); //AGREGUE
		free(patheando);
	}
	free(bloques);
	return 1;
}
void verBloques(char *path) {
	int* bloques = obtenerBloques(path);
	int cantidadDeBloques = cantidadBloques(path);
	for (int i = 0; i < cantidadDeBloques; i++)
		printf(" %d ", bloques[i]);
	printf("\n");
}
void veamoss(char *path) {
	printf("TAM: %d: \n", sizeArchivo(path));
	printf("CANTIDAD DE BLOQUES: %d \n", cantidadBloques(path));
	printf("VECTOR : ");
	int*bloques = obtenerBloques(path);
	for (int i = 0; i < cantidadBloques(path); i++)
		printf(" %d ", bloques[i]);
	printf("\n");
	free(bloques);
}
char* caminoBloque(int bloque) {
	int tamanioPath=strlen(config_get_string_value(config,"PUNTO_MONTAJE"))+1;
	char* patheando = malloc(tamanioPath);
	memcpy(patheando,config_get_string_value(config,"PUNTO_MONTAJE"),tamanioPath);
	string_append(&patheando, "/Bloques/");
	char*camino = malloc(strlen(patheando) + 1);
	strcpy(camino, patheando);
	char* numero = string_itoa(bloque);
	string_append(&camino, numero);
	string_append(&camino, ".bin");
	free(numero);
	free(patheando);
	return camino;
}
int garantizarLimites(char *path, size_t size, off_t offset) {
	int tamanioTotal = sizeArchivo(path);
	int c = tamanioTotal - size - offset;
	return (c >= 0) ? 1 : -1;

}

int garantizoExistencia(int bloque) {
	char *path = caminoBloque(bloque);
	FILE *fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return -1;
	else {
		fclose(fd);
		return 1;
	}
}

int escribirEnBloque(off_t offset, size_t size, int bloque, void *buffer) {
	if (garantizoExistencia(bloque) < 0)
		return -1;
	char *path = caminoBloque(bloque);
	//printf("Ruta bloque:\n%s\n", path);
	FILE *file = fopen(path, "r+");
	if (ftruncate(fileno(file), offset+size) != 0) {
			log_error(logger, "No se pudo garantizar tam bloque");
	}
	fclose(file);
	int fd = open(path, O_RDWR);
	struct stat sb;
	if (fstat(fd, &sb) == -1) {
		log_error(logger, "error!");
	}
	char* file_in_memory = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	char b[size];
	memcpy(b, buffer, size);
	for (int i = 0; i < size; i++)
		file_in_memory[i + offset] = b[i];
	free(path);
	munmap(file_in_memory, sb.st_size);
	return 1;
}

void mostrarElBloque(off_t offset, size_t size, int bloque) {
	char *path = caminoBloque(bloque);
	FILE *fd = fopen(path, "r");
	fseek(fd, offset, SEEK_SET);
	free(path);
	char *buffer = malloc(size + 1);
	fread((void *) buffer, 1, size, fd);
	buffer[size] = '\0';
	//printf("%s\n", buffer);
	fclose(fd);
	free(buffer);
}
void* leerElBloque(off_t offset, size_t size, int bloque) {
	char *path = caminoBloque(bloque);
	FILE *fd = fopen(path, "r");
	fseek(fd, offset, SEEK_SET);
	free(path);
	char *buffer = malloc(size);
	fread((void *) buffer, 1, size, fd);
	fclose(fd);
	return buffer;
}

devMDJ* retornarConBuffer(void* buffer, size_t size) {
	devMDJ *dev = malloc(sizeof(devMDJ));
	dev->error = 0;
	dev->rta = DATAOBTENIDA;
	dev->tamBuffer = size;
	dev->buffer = malloc(size);
	memcpy(dev->buffer, buffer, size);

	return dev;
}

devMDJ* obtenerDatos(char *path, off_t offset, size_t size) {
	if (verificarValidacion(path) < 0) {
		printMDJ("ERROR: 10001/30004 Path Inexistente");
		return retornarError(AIF);
	}
	if (offset > sizeArchivo(path)) {
		devMDJ* rtaCopada = retornarConBuffer("", 0); //ACA CAMBIE
		return rtaCopada;
	}
	if ((sizeArchivo(path) - offset) < size) {
		size = (sizeArchivo(path) - offset);
	}
	size_t tamanio = size;
	int bloqueActual = offset / TAM_BLOQUE;
	off_t desplazamiento = offset % TAM_BLOQUE;
	size_t pedacito = TAM_BLOQUE - desplazamiento;
	int* bloques = obtenerBloques(path);
	int cb = bloqueActual;
	usleep(config_get_int_value(config, "RETARDO"));
	void *buffer = malloc(size);
	if (size >= pedacito) {
		void*lec = leerElBloque(desplazamiento, pedacito, bloques[cb]);
		size -= pedacito;
		memcpy(buffer, lec, pedacito);
		cb++;
		free(lec); //CAMBIE ACA
		int contador = pedacito;
		while (size > 0) {
			if (size >= TAM_BLOQUE) {
				void*lectura = leerElBloque(0, TAM_BLOQUE, bloques[cb]);
				memcpy(buffer + contador, lectura, TAM_BLOQUE);
				size -= TAM_BLOQUE;
				contador += TAM_BLOQUE;
				cb++;
				free(lectura); //ACA CAMBIE
			} else {
				void *lect = leerElBloque(0, size, bloques[cb]);
				memcpy(buffer + contador, lect, size);
				free(lect);
				size = 0;
			}
		}
	}

	else {
		void* bufferBloque=leerElBloque(desplazamiento, size, bloques[cb]);
		memcpy(buffer,bufferBloque, size);
		free(bufferBloque);

	}

	free(bloques);
	devMDJ* rtaCopada = retornarConBuffer(buffer, tamanio); //ACA CAMBIE
	free(buffer);
	return rtaCopada;
}

devMDJ* borrarArchivo(char* path) {
	if (verificarValidacion(path) < 0) {
		printMDJ("ERROR: 60001: El archivo no existe. \n");
		return retornarError(AIB);

	}
	int *bloques = obtenerBloques(path);
	inicializarBloques(path);
	for (int i = 0; i < cantidadBloques(path); i++) {
		desSetear(bloques[i]);
	}
	char *camino = crearCamino(path);
	if (remove(camino) < 0) {
		free(camino);
		free(bloques);//th
		printMDJ("ERROR: 60001: El archivo no existe. \n");
		return retornarError(DIFICULTADESTECNICAS);
	} else {
		free(camino);
		free(bloques);//th
		return retornarExito();
	}
}

int cantidadCifras(int n) {
	int contador = 1;
	while (n / 10 > 0) {
		n /= 10;
		contador++;
	}
	return contador;
}

void crearFileMetadata(char *path, int tamanio, int* bloques,
		int cantidadBloques) {
	int n = cantidadCifras(tamanio);
	char tam[n];
	sprintf(tam, "%d", tamanio);
	char* blocks = intarraychars(bloques, cantidadBloques);
	crearNuevo(path, tam, blocks);
	free(blocks);

}
char* intarraychars(int* bloques, int cantidadDeBloques) {
	int contador = 4 + cantidadDeBloques;
	for (int i = 0; i < cantidadDeBloques; i++)
		contador += cantidadCifras(bloques[i]);
	char *blocks = malloc(contador);
	blocks[0] = '[';
	int contadorinterno = 1;
	for (int i = 0; i < cantidadDeBloques; i++) {
		sprintf(blocks + contadorinterno, "%d", bloques[i]);
		contadorinterno += cantidadCifras(bloques[i]);
		if (i < cantidadDeBloques - 1) {
			blocks[contadorinterno] = ',';
			contadorinterno++;
		}
	}
	blocks[contador - 4] = ']';
	blocks[contador - 3] = '\0';
	return blocks;
}
int cantidadBloques(char *path) {
	char* patheando = crearCamino(path);
	t_config* confi = config_create(patheando);
	char** array = config_get_array_value(confi, "BLOQUES");
	int contador;
	for (contador = 0; array[contador] != NULL; contador++)
		;
	free(patheando);
	config_destroy(confi);

	int i;//---------------------------------------------------------------MEM LEAK
	for (i = 0; array[i] != NULL; i++);
//	i--;
	for(int k=i;k>=0;k--)
		free(array[k]);

	free(array);
	return contador;
}

int* obtenerBloques(char *path) {
	//Si se usa se debe liberar los bloques
	char* patheando = crearCamino(path);
	t_config* confi = config_create(patheando);//----------------------------------------PROBLEMA CONFIG
	char** array = config_get_array_value(confi, "BLOQUES");
	int cbloques = cantidadBloques(path);
	int* bloques = malloc(sizeof(int) * cbloques);
	for (int i = 0; i < cbloques; i++)
		bloques[i] = atoi(array[i]);
	free(patheando);

	int i;
	for (i = 0; array[i] != NULL; i++);
//	i--;
	for(int k=i;k>=0;k--)
		free(array[k]);

	free(array);
	config_destroy(confi); //CAMBIO
	return bloques;
}
int sizeArchivo(char* path) {
	char*patheando = crearCamino(path);
	t_config* confi = config_create(patheando);
	int tamanio = config_get_int_value(confi, "TAMANIO");
	config_destroy(confi);
	free(patheando); //ACA CAMBIE

	return tamanio;
}

int agregarBloquesAlArchivo(char*path, int* newblocks, int cantnewBlocks) {
	int *blocks = obtenerBloques(path);
	int oldQuBlocks = cantidadBloques(path);
	//printf("Bloques viejos: \n");
	//for (int i = 0; i < oldQuBlocks; i++)
		//printf(" %d ", blocks[i]);
	//printf("\n");
	int nuevacantidad = oldQuBlocks + cantnewBlocks;
	//printf("cantidad vieja de bloques: %d\n", oldQuBlocks);

	blocks = realloc(blocks, nuevacantidad * sizeof(int));
	for (int i = 0; i < cantnewBlocks; i++) {
		blocks[i + oldQuBlocks] = newblocks[i];
	}
	char*nuevo = intarraychars(blocks, nuevacantidad);
	//printf("Bloques: %s \n", nuevo);
	modificarBloques(path, nuevo);
	free(blocks);
	free(nuevo);
	return 1;
}
void modificarBloques(char*path, char*bloques) {
	int tamanio = sizeArchivo(path);
	int n = cantidadCifras(tamanio);
	char tam[n];
	sprintf(tam, "%d", tamanio);
	modificar(path, tam, bloques);
}

void modificarTamArchivo(char*path, int tamanio) {
	int *blocks = obtenerBloques(path);
	int cbloques = cantidadBloques(path);
	char *bloquesenchar = intarraychars(blocks, cbloques);
	int n = cantidadCifras(tamanio);
	char tam[n];
	sprintf(tam, "%d", tamanio);
	modificar(path, tam, bloquesenchar);
	free(bloquesenchar);
	free(blocks);

}

void modificar(char*path, char* tamanio, char* bloques) {
	char* linea1 = string_from_format("TAMANIO=%s\n", tamanio);
	//printf("%s", linea1);
	char* linea2 = string_from_format("BLOQUES=%s\n", bloques);
	//printf("%s\n", linea2);
	char* patheando = crearCamino(path);
	int fd = open(patheando, O_RDWR);
	FILE*archivo = fopen(patheando, "w");
	if (ftruncate(fileno(archivo), strlen(linea1)+ strlen(linea2)) != 0) {
		log_error(logger, "No se pudo extender el archivo\n");
	}
	struct stat sb;
	if (fstat(fd, &sb) == -1) {
		log_error(logger, "error!");
	}
	//printf("El tamaño del archivo es: %d\n", sb.st_size);
	char* file_in_memory = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	for (int i = 0; i < strlen(linea1); i++) {
		file_in_memory[i] = linea1[i];
	}
	int lim = strlen(linea1);
	for (int i = 0; i < strlen(linea2); i++) {
		file_in_memory[i + lim] = linea2[i];
	}
	fclose(archivo);
	free(patheando);
	free(linea1);
	free(linea2);
	munmap(file_in_memory, sb.st_size);

}

void crearNuevo(char*path, char* tamanio, char* bloques) {
	char* patheando = crearCamino(path);
	FILE*archivo = fopen(patheando, "w");
	char* linea1 = string_from_format("TAMANIO=%s", tamanio);
	//printf("%s \n", linea1);
	char* linea2 = string_from_format("BLOQUES=%s", bloques);
	//printf("%s \n", linea2);
	fprintf(archivo, "%s\n%s\n", linea1, linea2);
	free(patheando);
	free(linea1);
	free(linea2);
	fclose(archivo);
}

devMDJ* guardarDatos(char *path, off_t offset, size_t size, void*buffer) {
	if (verificarValidacion(path) < 0) {
		return retornarError(AIF);
	}
	char* patheando = crearCamino(path);
	int tamanioArchivo = sizeArchivo(path);
//	printf("Estoy dentro de la funcion guardarDatos.\n "
//			"Tamaño del archivo %d\n", tamanioArchivo);
	int *bloques = obtenerBloques(path);
	int cantidadDeBloques = cantidadBloques(path);
	int faltante = 0;
	if (garantizarLimites(path, size, offset) < 0) {
		//printf("\nSe debe extender el archivo\n");
		faltante = size + offset - tamanioArchivo;
		int v = tamanioArchivo % TAM_BLOQUE;
		if (v == 0)
			v = TAM_BLOQUE;
		int pedacitoSinAsignar = TAM_BLOQUE - v;
		//printf("El tamaño del archivo deberia ser %d \n",
			//	tamanioArchivo + faltante);
		modificarTamArchivo(path, tamanioArchivo + faltante);
		if (pedacitoSinAsignar < faltante) {
			faltante -= pedacitoSinAsignar;
			int cantidadBloquesFaltantes = faltante / TAM_BLOQUE;
			if ((faltante % TAM_BLOQUE) != 0)
				cantidadBloquesFaltantes++;
			//printf("Cantidad de bloques faltantes: %d\n",
				//	cantidadBloquesFaltantes);
			int *bufferints = reservarBloquesRequeridos(
					cantidadBloquesFaltantes);
			if (!bufferints) {
				free(patheando);
				free(path);
				free(bloques);
				log_error(logger,
						"No se pudierons asignar mas bloques al archivo! CRISISS\n");
				return retornarError(EIA);
			}
			//printf("Bloques faltantes asignados \n");
			//for (int i = 0; i < cantidadBloquesFaltantes; i++) {
			//	printf(" %d ", bufferints[i]);
		//	}
			agregarBloquesAlArchivo(path, bufferints, cantidadBloquesFaltantes);
			free(bloques);
			bloques = obtenerBloques(path);
			//printf("estoy aca en agregarBloquesAlArchivo\n");
			free(bufferints);
			cantidadDeBloques += cantidadBloquesFaltantes;
			//printf(
				//	"Nos asignan nuevos bloques!\nEn total los bloques son: \n ");
			//for (int i = 0; i < cantidadDeBloques; i++)
				//printf("% d ", bloques[i]);
			//printf("\n");
		}
	}
	int indice = offset / TAM_BLOQUE;
	//printf("INDICE %d\n", indice);
	//printf("bloque que toca %d ", bloques[indice]);
	int desplazamiento = offset % TAM_BLOQUE;
	int pedacito = TAM_BLOQUE - desplazamiento;
	if (size > pedacito) {
		//printf("Se va a escribir el bloque: %d\n", bloques[indice]);
		escribirEnBloque(desplazamiento, pedacito, bloques[indice], buffer);
		char ab[pedacito+1];
		memcpy(ab,buffer,pedacito);
		ab[pedacito]='\0';
		int desp = pedacito;
		size -= pedacito;
		indice++;
		while (size > 0) {
			if (size > TAM_BLOQUE) {
				//printf("Se va a escribir el bloque: %d\n", bloques[indice]);
				escribirEnBloque(0, TAM_BLOQUE, bloques[indice], buffer + desp);
				char abc[TAM_BLOQUE+1];
						memcpy(abc,buffer,TAM_BLOQUE);
						abc[TAM_BLOQUE]='\0';
				indice++;
				size -= TAM_BLOQUE;
				desp += TAM_BLOQUE;
			} else {
				//printf("Se va a escribir el bloque: %d\n", bloques[indice]);
				escribirEnBloque(0, size, bloques[indice], buffer + desp);
				char abcd[size+1];
				memcpy(abcd,buffer,size);
				abcd[size]='\0';
				size = 0;
			}
		}

	} else {
		escribirEnBloque(desplazamiento, size, bloques[indice], buffer);
	}
	free(patheando);//agg
	free(bloques);
	return retornarExito();
}

