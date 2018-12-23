#include "planificadores.h"
#define SAFA_OPERATIVO cpu==1 && dam==1

int socketDiego;
extern sem_t diego_me_hablo;
extern sem_t el_diego_ya_hizo_lo_suyo;
extern pthread_mutex_t lock;
extern t_config* config;
extern int validacionSAFA;
t_list* colaEXIT;
extern t_list* colaREADY;
extern t_list* colaBLOQ;
extern t_log* logger;
extern sem_t sem_planificar;
extern sem_t sem_disponibilidad;
extern sem_t sem_diego_y_cpu_conectados;
extern sem_t multiprogramacion;
extern CPU procesadores[MAX_CPU]; //esto tambien va ser configurado
extern int gradoMultiprogramacion;
extern int procesosTerminados;
extern sem_t sem_cerrar_memoria;
extern t_list*colaLIBERAR;

int cantBloqueantes;
int cantEjecutadas;

extern int volvioUnDTB;
extern int procesoContado;
extern time_t tiempoEnvio;
time_t tiempoLlegada;


#define PORT 8000   // puerto en el que escuchamos

   void conexiones(void)
   {
	   cantBloqueantes=0;
	   cantEjecutadas=0;
	   colaEXIT=list_create();


	   printf("\n\x1b[33m------Planificador SAFA Encendido ------\n-Algoritmo: %s\n-Estado: Corrupto \n \nEsperando conexiones...\n\x1b[0m",config_get_string_value(config,"ALGORITMO"));
	   sem_init(&diego_me_hablo,0,0);
	   sem_init(&el_diego_ya_hizo_lo_suyo,0,0);

       fd_set master;   // conjunto maestro de descriptores de fichero
       fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
       struct sockaddr_in myaddr;     // dirección del servidor
       struct sockaddr_in remoteaddr; // dirección del cliente
       int fdmax;        // número máximo de descriptores de fichero
       int listener;     // descriptor de socket a la escucha
       int newfd;        // descriptor de socket de nueva conexión aceptada
       int nbytes;
       int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
       unsigned int addrlen;
       int i;
       int cpu=0;
    	int dam=0;
       FD_ZERO(&master);    // borra los conjuntos maestro y temporal
       FD_ZERO(&read_fds);
       // obtener socket a la escucha
       if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
           perror("socket");
           exit(1);
       }
       // obviar el mensaje "address already in use" (la dirección ya se está usando)
       if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                                                           sizeof(int)) == -1) {
           perror("setsockopt");
           exit(1);
       }
       // enlazar
       myaddr.sin_family = AF_INET;
       myaddr.sin_addr.s_addr = INADDR_ANY; //aca vamos a poner la  ip
       myaddr.sin_port = htons(config_get_int_value(config,"PUERTO"));
       memset(&(myaddr.sin_zero), '\0', 8);
       if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
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
       for(;;) {
           read_fds = master; // cópialo
           if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
               perror("select");
               exit(1);
           }
           // explorar conexiones existentes en busca de datos que leer
           for(i = 0; i <= fdmax; i++) {
               if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
                   if (i == listener) {
                       // gestionar nuevas conexiones
                       addrlen = sizeof(remoteaddr);
                       if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,&addrlen)) == -1) {
                           perror("accept");
                       } else {
                           FD_SET(newfd, &master); // añadir al conjunto maestro
                           if (newfd > fdmax) {    // actualizar el máximo
                               fdmax = newfd;
                           }
                          //trato con nuevas conexiones
                           void*paquete=malloc(4);
                           int identificador=0;
                           recv(newfd,paquete,4,MSG_WAITALL); //cpu me manda un 1 y dam un 2
                           memcpy(&identificador,paquete,4);
                           free(paquete);
                           if(identificador==1){
                        	   procesadores[cpu].conexion=newfd;
                        	   procesadores[cpu].disponibilidad=1;
                        	   sem_post(&sem_disponibilidad);
                        	   cpu++;
                        	   log_info(logger,"se ha conectado una cpu");
                           }
                           if(identificador==2){
                        	   dam++;
                        	   log_info(logger,"se ha conectado el dam");
                           }
                           if(SAFA_OPERATIVO){
                        	  printf("\x1b[33m-Estado: Operativo \n \x1b[0m");
                        	   validacionSAFA=1;
                        	   sem_post(&sem_diego_y_cpu_conectados);
                           }


                       }
                   } else {
                       // gestionar datos de un cliente
                	   int buffer;
                       if ((nbytes = recv(i,&buffer,4,0)) <= 0) {
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

                    	   	   switch(buffer){

                    	   	   case 1:	;//llega dtb del cpu;
                    	   	   	   log_info(logger,"\n\x1b[33mAtendiendo CPU\n\x1b[0m");
                    	   	   	   log_info(logger,"el buffer que me enviaron es %d",buffer);
                    	   	   	   log_info(logger,"osea que recibi algo de una cpu");
                    	   		   int tamanioPaq=getHeaderSendHandshake(i,logger);
                    	   		   log_info(logger,"lo que me mando el cpu pesa %d bytes",tamanioPaq);
                    	   		   void* paqueteCPU=malloc(tamanioPaq);
                    	   		   getPayload(paqueteCPU,tamanioPaq,i,logger);
                    	   		   //volver al cpu disponible

                    	   		   log_info(logger,"un procesador deberia estar disponible");

                    	   		   DTB*dtb=malloc(sizeof(DTB));
                    	   		   log_info(logger,"se va a cargar el dtb enviado por el cpu");
                    	   		   cargarDTBSinInformar(dtb,paqueteCPU,logger);
                    	   		   free(paqueteCPU);

                    	   		   //parte para metricas
                    	   		   if(procesoContado==dtb->ID){
                    	   			   tiempoLlegada=time(NULL);
                    	   			   aniadirAColaDeTiempos(tiempoEnvio,tiempoLlegada);
                    	   			   volvioUnDTB++;
                    	   		   }
                    	   		   //tratado de cpus
                    	   		   if(dtb->FlagInic==0){//si es un dummy

                    	   		   }else if(dtb->FlagInic==2){

                    	   		   }else
                    	   		   {
                    	   			  int pos=0;

                    	   			 if(estaElProcesoEnEXEC(dtb->ID,&pos,logger)){
                    	   				DTB*dtbEXEC=malloc(sizeof(DTB));
                    	   				cargarDTBSinInformar(dtbEXEC,conseguirProcesoEnEXEC(dtb->ID,logger),logger);
                    	   				int sentenciasNEW=dtb->PC-dtbEXEC->PC;
                    	   			 pthread_mutex_lock(&lock);
                    	   				actualizarNEW(sentenciasNEW);
                    	   			 pthread_mutex_unlock(&lock);
                    	   				liberar_lista(dtbEXEC->tablaDeDirecciones);
                    	   				list_destroy(dtbEXEC->tablaDeDirecciones);
                    	   				free(dtbEXEC->path);
                    	   				free(dtbEXEC);
                    	   			 }

                    	   			   switch(dtb->estado){
                    	   			   case EXEC://termino el quantum
                    	   				   dtb->estado=READY;
                    	   				   dtb->quantum=config_get_int_value(config,"QUANTUM");

                    	   				 pthread_mutex_lock(&lock);
                    	   				 if(estaElProcesoEnEXEC(dtb->ID,&pos,logger)){
                    	   				 	destruirProcesoEnEXEC(dtb->ID,logger);
                    	   				   list_add(colaREADY,dtbAPaquete(dtb));
                    	   				   sem_post(&sem_planificar);
                    	   				 }
                    	   				 pthread_mutex_unlock(&lock);

                    	   				   log_info(logger,"el proceso %d fue aniadido a la cola de READY",dtb->ID);
                    	   				   break;

                    	   			   case BLOQ: //operacion bloqueante

                    	   				   if(dtb->quantum==0||!string_equals_ignore_case("VRR",config_get_string_value(config,"ALGORITMO")))
                    	   					   dtb->quantum=config_get_int_value(config,"QUANTUM");

                    	   				pthread_mutex_lock(&lock);
                    	   				if(estaElProcesoEnEXEC(dtb->ID,&pos,logger)){
                    	   					destruirProcesoEnEXEC(dtb->ID,logger);
                    	   				   list_add(colaBLOQ,dtbAPaquete(dtb));
                    	   			   }
                    	   				pthread_mutex_unlock(&lock);

                    	   				   log_info(logger,"el proceso %d fue aniadido a la cola de BLOQ",dtb->ID);
                    	   				   break;

                    	   			   case EXIT:// mandar a la mierda el proceso

                    	   				 if(estaElProcesoEnEXEC(dtb->ID,&pos,logger)){
                    	   				pthread_mutex_lock(&lock);
                    	   				destruirProcesoEnEXEC(dtb->ID,logger);
                    	   				  gradoMultiprogramacion--;
                    	   				  procesosTerminados++;
                    	   				  list_add(colaEXIT,dtbAPaquete(dtb));
                    	   				  sem_post(&multiprogramacion);
                    	   				  actualizarRecursos();
                    	   				pthread_mutex_unlock(&lock);

                    	   				list_add(colaLIBERAR,dtbAPaquete(dtb));
                    	   				sem_post(&sem_cerrar_memoria); //DELEGAR TRABAJO A HILO 7

                    	   				 }
                    	   				   log_info(logger,"el proceso %d se mando a EXIT",dtb->ID);
                    	   				   break;

                    	   			   default:
                    	   				   log_info(logger,"esto significa que tom no me mando algo como bloq ready o exit");
                    	   			   }

                    	   		   }

                    			   int x;
                    	           for(x=0;procesadores[x].conexion!=i;x++);
                    	           procesadores[x].disponibilidad=1; //hago que se le puedan mandar cosas
                    	           sem_post(&sem_disponibilidad);

                    	   	       liberar_lista(dtb->tablaDeDirecciones);
               	   				   list_destroy(dtb->tablaDeDirecciones);
               	   				   free(dtb->path);
               	   				   free(dtb);
                    	   		   break;

                    	   	   case 2:	;//dam me hablo;
                    	   	   	   log_info(logger,"\n\x1b[33mAtendiendo DAM\n\x1b[0m");
                    	   	   	   socketDiego=i;
                    	   		   sem_post(&diego_me_hablo);
                    	   		   //espero a que el diego reciba lo que tiene que recibir
                    	   		   sem_wait(&el_diego_ya_hizo_lo_suyo);
                    	   		   break;

                    	   	   case 3: //wait
                    	   		   ;
                    	   		 pthread_mutex_lock(&lock);
                    	   		  operacionWait(i);
                    	   		 pthread_mutex_unlock(&lock);


                    	   		   break;

                    	   	   case 4://signal
                    	   	   ;
                    	   	 pthread_mutex_lock(&lock);
                    	   	   operacionSignal(i);
                    	   	 pthread_mutex_unlock(&lock);


                    	   	   break;

                    	   	   default:	;//nada
                    	 	 }

                       }
                   } // Esto es ¡TAN FEO!
               }
           }
       }

   }

