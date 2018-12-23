
#include "planificadores.h"
extern t_config*config;
extern t_log*logger;
extern pthread_mutex_t lock;
extern t_list* colaNEW;
extern t_list* colaEXEC;
extern t_list* colaREADY;
extern t_list*colaBLOQ;
extern t_list*colaEXIT;
extern sem_t  sem_plp;
extern sem_t sem_liberar_dir;
extern char* dirEscript;
extern sem_t multiprogramacion;
extern int gradoMultiprogramacion;
extern int procesosTerminados;
int salida =0;

void consola() {
	char * linea;
	char * path;
	char* idDTB;

	while (1) {
		printf("\n");
		linea = readline(">>");

		if (linea)
			add_history(linea);

		if (!strcmp(linea, "ls")) {
			printf("Aca estan los archivos  \n");
		}

		if (!strncmp(linea, "ejecutar ", 9)) {
			path = string_substring_from(linea, 9);
			string_trim(&path);  //Para poder hacer el if siguiente
			if (!strcmp(path, "")) { //Si se escribio mal hace el if
				printSafa(
						"Ejecutar toma como parametro un archivo a ejecutar. Por ejemplo: ejecutar archivo\n");
						free(path);
			} else {
				//gradoMultiprogramacion++; agregar esto en algun lado
					printSafa("Ejecutando el archivo en ");
				printf("%s\n", path);

					dirEscript = malloc(strlen(path)+1);
						memcpy(dirEscript,path,strlen(path)+1);
							free(path);

					sem_post(&sem_plp); //Se manda a ejecutar el plp.c

					sem_wait(&sem_liberar_dir); //Se espera a que plp.c termine de crear el DTB
			}
		}

		if (!strncmp(linea, "status ", 7)) {
				idDTB = string_substring_from(linea, 7);
					string_trim(&idDTB);  //Para poder hacer el if siguiente
					if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
						system("clear");
								pthread_mutex_lock(&lock);
								status();
								pthread_mutex_unlock(&lock);
					} else {
						system("clear");


							//mostrar datos de un dtb
						pthread_mutex_lock(&lock);
							mostrarDatosDTB(idDTB,logger);
						pthread_mutex_unlock(&lock);

					}
					free(idDTB);
				}

	if (!strcmp(linea, "status")) { //hacer caso con parametro
		system("clear");
		pthread_mutex_lock(&lock);
		status();
		pthread_mutex_unlock(&lock);
	}

	if (!strncmp(linea, "finalizar ", 10)) {
					idDTB = string_substring_from(linea, 10);
						string_trim(&idDTB);  //Para poder hacer el if siguiente
						if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
							system("clear");
								printSafa("Por favor escriba el id del proceso que quiere finalizar");
						} else {
							system("clear");
								//mostrar datos de un dtb

								finalizar(idDTB,logger);
							pthread_mutex_lock(&lock);
								actualizarRecursos();
							pthread_mutex_unlock(&lock);
						}
						free(idDTB);
					}

	if (!strncmp(linea, "metricas ", 9)) {
						idDTB = string_substring_from(linea, 9);
							string_trim(&idDTB);  //Para poder hacer el if siguiente
							if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
								system("clear");
									printSafa("Por favor escriba el id del proceso que quiere finalizar");
							} else {
								system("clear");
									//mostrar datos de un dtb
								pthread_mutex_lock(&lock);
									mostrarMetricasDTB(idDTB);
								pthread_mutex_unlock(&lock);
							}
							free(idDTB);
						}
	if (!strncmp(linea, "algoritmo ", 10)) {
						idDTB = string_substring_from(linea, 10);
							string_trim(&idDTB);  //Para poder hacer el if siguiente
							if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
								system("clear");
									printSafa("Por favor escriba el algoritmo al que quiera cambiar");
							} else {
								system("clear");
									//mostrar datos de un dtb
								pthread_mutex_lock(&lock);
									cambiarAlgoritmo(idDTB);
								pthread_mutex_unlock(&lock);
							}
							free(idDTB);
						}
	if (!strncmp(linea, "quantum ", 8)) {
							idDTB = string_substring_from(linea, 8);
								string_trim(&idDTB);  //Para poder hacer el if siguiente
								if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
									system("clear");
										printSafa("Por favor escriba el quantum con el que desea ejecutar");
								} else {
									system("clear");
										//mostrar datos de un dtb
									pthread_mutex_lock(&lock);
										cambiarQuantum(idDTB);
									pthread_mutex_unlock(&lock);
								}
								free(idDTB);
							}

	if (!strncmp(linea, "retardo ", 8)) {
								idDTB = string_substring_from(linea, 8);
									string_trim(&idDTB);  //Para poder hacer el if siguiente
									if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
										system("clear");
											printSafa("Por favor escriba el quantum con el que desea ejecutar");
									} else {
										system("clear");
											//mostrar datos de un dtb
										pthread_mutex_lock(&lock);
											cambiarRetardo(idDTB);
										pthread_mutex_unlock(&lock);
									}
									free(idDTB);
								}
	if (!strncmp(linea, "multiprogramacion ", 18)) {
								idDTB = string_substring_from(linea, 18);
									string_trim(&idDTB);  //Para poder hacer el if siguiente
									if (!strcmp(idDTB, "")) { //Si se escribio mal hace el if
										system("clear");
											printSafa("Por favor escriba el quantum con el que desea ejecutar");
									} else {
										system("clear");
											//mostrar datos de un dtb
										pthread_mutex_lock(&lock);
											cambiarGradoMP(idDTB);
										pthread_mutex_unlock(&lock);
									}
									free(idDTB);
								}

	if (!strcmp(linea, "metricas")) {

		system("clear");
		pthread_mutex_lock(&lock);
			mostrarMetricasSistema();
		pthread_mutex_unlock(&lock);
	}

	if (!strcmp(linea, "clear")) {
		system("clear");
	}

	if (!strcmp(linea, "exit")) {
		free(linea);
		salida = 1;
		sem_post(&sem_plp);
		sleep(1);
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
