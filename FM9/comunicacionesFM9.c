#include "gs.h"
#include "funcionesFM9.h"
extern t_log* loggerFM9;
int socketDiego;
int procesadores[10]; //OJO DESPUES QUIZA HAY QUE CAMBIARLO
extern t_config*config;
extern sem_t mutexTabla;
extern char* storage;
extern int proximoID;

void coms() {
	printf("Esperando conexiones...");
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
	int cpu = 0;
	int dam = 0;
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
	//myaddr.sin_port = htons(config_get_int_value(config,"PUERTO"));
	myaddr.sin_port = htons(8003);
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
						void*paquete = malloc(4);
						int identificador = 0;
						recv(newfd, paquete, 4, MSG_WAITALL); //cpu me manda un 1 y dam un 2
						memcpy(&identificador, paquete, 4);
						free(paquete);
						if (identificador == 1) {
							procesadores[cpu] = newfd;
							cpu++;
							log_info(loggerFM9, "se ha conectado una cpu");
						}
						if (identificador == 2) {
							dam++;
							log_info(loggerFM9, "se ha conectado el dam");
						}
//						if(SAFA_OPERATIVO){
//							printf("\x1b[33m-Estado: Operativo \n \x1b[0m");
//							validacionSAFA=1;
//							sem_post(&sem_diego_y_cpu_conectados);
//						}

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
						// tratamos al cliente que se nos conecto
						switch (buffer) {

						case 1:
							; //cpu me hablo;
							log_info(loggerFM9,
									"el buffer que me enviaron es %d", buffer);
							log_info(loggerFM9,
									"osea que recibi algo de una cpu");
							int x;
							for (x = 0; procesadores[x] != i; x++)
								;
							void*paquete = malloc(16); //esto deberian ser 12, no lo cambio porque ahora no vale la pena
							int pid = 0;
							int seg;
							int offset;
							int accionPedida;
							int tamanioDatos = 0;
							//	char*datos;

							getPayload(paquete, 16, procesadores[x], loggerFM9);
							memcpy(&seg, paquete, 4);
							memcpy(&offset, paquete + 4, 4);
							memcpy(&pid, paquete + 4 + 4, 4);
							memcpy(&accionPedida, paquete + 4 + 4 + 4, 4);
							printf("ACCION PEDIDA: %d\n", accionPedida);
							switch (accionPedida) {
							case 1:
								//busco en memoria y devuelvo
								printf(" pid.ID,offset %d %d %d", pid, seg,
										offset);
								sem_wait(&mutexTabla);
								//Ojo tambien manda la linea al procesador buscarLineaPedida
								int lineaEncontrada = buscarLineaPedida(pid,
										seg, offset, procesadores[x]);
								sem_post(&mutexTabla);
								break;
							case 2: //asignar
								recv(procesadores[x], &tamanioDatos, 4,
										MSG_WAITALL);
								char* datos = malloc(tamanioDatos);
								recv(procesadores[x], datos, tamanioDatos,
										MSG_WAITALL);
								offset -= 1; //le resto uno a offset porque yo cuento las lineas desde 0
								cambiarPosicion(pid, seg, offset, datos, procesadores[x]);
								log_info(loggerFM9, "fin rutina de asignar");
								//free(datos); adentro de cambiarPosicion
								break;
							case 3: //close
								liberarMemoria(pid, seg);
								break;
							}
							accionPedida = 0;
							free(paquete);
							break;

						case 2:
							; //dam me hablo;
							log_info(loggerFM9,
									"el entero que me enviaron es %d", buffer);
							log_info(loggerFM9,
									"osea que me llego algo del dam");
							socketDiego = i;
							//aca tendriamos que recibir el paquete que envia el DAM con la posta
							void* panela = malloc(12); //tres int

							if (recv(socketDiego, panela, 12, MSG_WAITALL)
									> 0) {
								log_info(loggerFM9,
										"Recibi con exito la panela");
								int accion = 0;
								int cantLineas = 0;
								memcpy(&accion, panela, 4);
								memcpy(&cantLineas, panela + 4, 4);
								//		log_info(loggerFM9, "la accion es de tipo %d",
								//					accion);
								//		log_info(loggerFM9,"la cantidad de lineas son %d",cantLineas);

							} else
								log_error(loggerFM9,
										"Erre el gol y no recibi la panela");

							int rta = realizarAccion(panela); //si es MDJ->FM9 devuelve lo que lo identifica
							if (rta == -1) { //ERROR
								log_error(loggerFM9,
										"ERROR: 10002 No espacio"); //si pasa esto es que no hay memoria o algo fallo ((WIP))
								int no = 0;
								send(socketDiego, &no, 4, 0);
							} else if (rta == FLUSH) {
								int oka = 1;
								send(socketDiego, &oka, 4, 0); //da el ok para recibir las lineas
								enviarMemoriaADiego(socketDiego, panela);
							} else if (rta == -3) {
								printf("Borrado\n");
							} else {
								int baseaux = rta;
								int nro = 0;
								int PID = 0;
								memcpy(&nro, panela + 4, 4);
								memcpy(&PID, panela + 8, 4);
								printf("Lineas: %d\n", nro);
								int ok = 1;
								send(socketDiego, &ok, 4, 0); //da el ok para recibir las lineas
								recibirYGuardarLineas(socketDiego, nro, rta,
										PID); //rta tiene la base del segmento

								if (devolverModo(config) == SEGPAG
										|| devolverModo(config) == SEG) {
									log_info(loggerFM9,
											"Cambiando la variable RTA");
									memcpy(&rta, &proximoID, 4); //}//Si es SEG/SEGPAG cambia
								}

								send(socketDiego, &rta, 4, 0); //Para que despues el safa se lo agg al DTB

								sem_wait(&mutexTabla);
								actualizarListas(baseaux, panela);

								if (devolverModo(config) == SEGPAG
										|| devolverModo(config) == SEG) {
									verArrayBits();
								} else {
									TI_mostrarEstadisticas();
								}

								sem_post(&mutexTabla);

							}
							//if(devolverModo(config)==SEGPAG){ //lo dejo por aca ahora.
							//antes estaba aca actualizar listas

							free(panela);
							break;

						default:
							printf("ojala no entre aca\n");		//nada
						}

					}
				} // Esto es ¡TAN FEO!
			}
		}
	}

}

/*CASE VIEJO DE ACTUALIZAR EL DAM EN EL SELECT
 * 						case 2:	;//dam me hablo;
 log_info(loggerFM9,"el buffer que me enviaron es %d",buffer);
 log_info(loggerFM9,"osea que me llego algo del dam");
 socketDiego=i;
 //aca tendriamos que recibir el paquete que envia el DAM con la posta

 //lo siguiente es hardcodeado para probar xd
 void* panela = malloc(8);//dos int

 int accion=1;
 int nro = 2;//dos lineas el archivo.
 memcpy(panela,&accion,sizeof(int));
 memcpy(panela+4,&nro,sizeof(int));
 //----------------------------------------------------
 int rta = realizarAccion(panela); //si es MDJ->FM9 devuelve base
 if(rta == -1){
 printf("no espacio\n");//si pasa esto es que no hay memoria o algo fallo ((WIP))
 send(socketDiego,"NO",3,0);}
 else{
 send(socketDiego,"OK",3,0);//da el ok para recibir las lineas
 char*bufferREC = malloc(32);//este seria el tam maximo
 for(int k=0;k<nro;k++){
 recv(socketDiego,bufferREC,13,MSG_WAITALL);
 memcpy(storage+(rta+k)*32,bufferREC,32);
 setear(rta+k);
 sem_wait(&mutexTabla);
 actualizarListas(rta,panela);
 sem_post(&mutexTabla);
 }}



 free(panela);
 break;
 */
