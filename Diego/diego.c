#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include"comunicacionMDJ.h"
#include "operacionesDAM.h"
DTB* dummy;
t_log *logger;
t_config*config;
int diego;
int safa;
int fm9;
int mdj_a_fm9;
//ip y puerto de escucha "127.0.0.1""8001"
int main(int argc, char *argv[]) {
	//pthread_t h_finalizarProcesos;//hilo
	//pthread_create(&h_finalizarProcesos, NULL, (void *) finalizar, NULL);
	//pthread_detach(h_finalizarProcesos);

	printf(
			"\n\x1b[35mDiego: Uhh quien me desperto wuacho estoy re duro...\n\n*bostezo*\n\nDiego: Hora de manejar unas cuantas conexiones..\n\x1b[0m\n"); //color del diego

	logger = log_create("diego.log", "diego", 1, 0);
	config =
			config_create(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/DAM.txt");

	//quien soy
	diego = 2; //identificador para las demas partes, antes de cualquier mensaje les notifico quien soy
	void*soydam = malloc(4);
	int dam = 2;
	mdj_a_fm9 = 1;
	int flush = 2;

	safa = connect_to_server(config_get_string_value(config, "IP_SAFA"),
			config_get_string_value(config, "PUERTO_SAFA"), logger);
	log_info(logger, "conectado al safa");

	memcpy(soydam, &dam, 4);
	send(safa, soydam, 4, 0);
	//le aviso al safa que soy el dam

	//conectar al fm9
	fm9 = connect_to_server(config_get_string_value(config, "IP_FM9"),
			config_get_string_value(config, "PUERTO_FM9"), logger);

	memcpy(soydam, &dam, 4);
	send(fm9, soydam, 4, 0);
	log_info(logger, "conectado al fm9");
	//conectar al mdj
	int mdj = connect_to_server(config_get_string_value(config, "IP_MDJ"),
			config_get_string_value(config, "PUERTO_MDJ"), logger);

	log_info(logger, "conectado al mdj");
	free(soydam);

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	struct sockaddr_in myaddr;     // dirección del servidor
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
	int nbytes;
	int yes = 1;        // para setsockopt() SO_REUSEADDR, más abajo
	unsigned int addrlen;
	int i;
	int recibidoDelFM9;

	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY; //aca vamos a poner la  ip
	myaddr.sin_port = htons(8001);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	// escuchar
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste
	// bucle principal
	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {    // actualizar el máximo
							fdmax = newfd;
						}
						//trato con nuevas conexiones
						log_info(logger, "se conecto una cpu!");

					}
				} else {
					// gestionar datos de un cliente
					int buffer;
					if ((nbytes = recv(i, &buffer, 4, 0)) <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						sleep(1); // RETARDO DAM--------------------------------------------HACER EL RETARDO SEGUN EL CONF

						switch (buffer) {

						case 1:
							; //recibir dummy
							buscardummy(buffer, i, mdj);
							break;

						case 2:
							; // caso abrir

							abrirEnMemoria(i,mdj);
							break;

						case 3:
							; //caso  flush
							hacerFlush(i,mdj);
							break;

						case 4:
							; //caso crear
							crearEnFS(i,mdj);
							break;

						case 5:
							; //caso borrar
							borrarEnFS(i,mdj);
							break;

						case 6:
							; //siguientes casos
							break;
						case 7:
							;
							break;
							//hacer panela con (ACCION(3),MEMORY POINTER,PID),, Acciones es un numero3. en ese orden
						default:
							; //nada
						}
					}
				} // Esto es ¡TAN FEO!
			}
		}
	}

	config_destroy(config);
	return 0;
}

/*
test con mdj
#include"comunicacionMDJ.h"
#include "operacionesDAM.h"
DTB* dummy;
t_log *logger;
t_config*config;
int main(int argc, char *argv[]) {

	logger = log_create("diego.log", "diego", 1, 0);
	config =
			config_create(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/DAM.txt");
	int mdj = connect_to_server(config_get_string_value(config, "IP_MDJ"),
			config_get_string_value(config, "PUERTO_MDJ"), logger);
	int tam;
	void* buffer=obtenerArchivo("y.txt",mdj,&tam);
}

 * */
