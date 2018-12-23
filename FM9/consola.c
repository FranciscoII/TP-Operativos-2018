#include "gs.h"
#include "funcionesFM9.h"
extern sem_t cerrarFM9;
extern sem_t mutexTabla;
int salida =0;

void consola() {
	char * linea;
	char * path;
	int PID;
	printf("Consola inicializada\n");
	while (1) {
		linea = readline("--->");

		if (linea)
			add_history(linea);

		if (!strncmp(linea, "dump ", 5)) {
			path = string_substring_from(linea, 5);
			string_trim(&path);  //Para poder hacer el if siguiente
			if (!strcmp(path, "")) { //Si se escribio mal hace el if
				printf(
						"Dump toma como parametro un DTB amigazo. Por ejemplo: dump 1001\n");
			} else {
				PID = atoi(path);
				system("clear");
				sem_wait(&mutexTabla);
				informarLista(PID);
				sem_post(&mutexTabla);
				printf("\n", path);

//					dirEscript = malloc(strlen(path)+1);
//						memcpy(dirEscript,path,strlen(path)+1);
							free(path);

			}
		}

	if (!strcmp(linea, "status")) {
		printf("status\n");
	}

	if (!strcmp(linea, "exit")) {
		free(linea);
		salida = 1;
		sleep(1);
		sem_post(&cerrarFM9);
		return;
	}
	free(linea);
}
}

/* ESTO HAY QUE RECONOCER:
 ejecutar
 status
 finalizar
 metricas*/
