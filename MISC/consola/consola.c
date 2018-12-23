#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>



void main() {
	char * linea;
	char * NombreArchivo;

	while (1) {
		linea = readline(">>");

		if (linea)
			add_history(linea);

		if (!strcmp(linea, "ls")) {
			printf("Aca estan los archivos xd  \n");
		}

		if (!strncmp(linea, "ejecutar ", 8)) {
			NombreArchivo = string_substring_from(linea, 8);
			string_trim(&NombreArchivo);  //Para poder hacer el if siguiente
			if (!strcmp(NombreArchivo, "")) {
				printf(
						"Ejecutar toma como parametro un archivo a ejecutar. Por ejemplo: ejecutar archivo\n");
			} else {
				printf("Ejecutando el archivo %s...\n", NombreArchivo); //Cambiar esto en el futuro, se va a pasar path no el nombre del archivo
				//Aca se manda a crear el DTB
			}
		}
	}

	if (!strcmp(linea, "status")) {
		printf("stus\n");
	}
	if (!strcmp(linea, "finalizar")) {
		printf("fin\n");
	}
	if (!strcmp(linea, "metricas")) {
		printf("metri\n");
	}

	if (!strcmp(linea, "clear")) {
		system("clear");
	}

	if (!strcmp(linea, "exit")) {
		free(linea);
		break;
	}
	free(linea);
}
}
/* ESTO HAY QUE RECONOCER:
 ejecutar
 status
 finalizar
 metricas*/
