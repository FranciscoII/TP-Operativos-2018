#include"gs.h"

void exit_gracefully(t_log *log, int return_nr) {
	log_destroy(log);
	exit(return_nr);
}
void exit_with_error(t_log *log, int socket, char *error_msg, void *buffer) {
	if (buffer != NULL) {
		free(buffer);
	}
	log_error(log, error_msg);
	close(socket);
	exit_gracefully(log, 1);
}

void recepcionMensaje(int receiver, t_log*logger, char*mensaje) {
	if (receiver < 0) {
		log_error(logger, "no se pudo recibir el mensaje");
	} else {
		log_info(logger, "mensaje recibido %s", mensaje);
	}
}

void enviar(int socket_cliente, void *dato, t_log *logger, size_t tamdetipo) {
	void* buf = malloc(sizeof(tamdetipo));
	memcpy(buf, dato, sizeof(tamdetipo));
	int r = send(socket_cliente, buf, sizeof(tamdetipo), 0);
	free(buf);
	if (r < 0) {
		log_error(logger, "No se pudo enviar");
		close(socket_cliente);

	} else {
		log_info(logger, "Se pudo enviar");
	}
}

void recibir(int socket_cliente, t_log *logger, void *dato, size_t tamTipo) {
	int bytes_recibidos = recv(socket_cliente, dato, sizeof(tamTipo), 0);
	if (bytes_recibidos < 0) {
		log_error(logger, "No se pudo recibir :c");
		close(socket_cliente);
		exit(1);
	} else {
		log_info(logger, "Se pudo recibir");
	}
}

void enviarMensaje(int socket_cliente, t_log *logger, char *mensaje) {
	int tamanio_Mens = strlen(mensaje) + 1;
	void *buffer = malloc(4 + tamanio_Mens * sizeof(char));
	memcpy(buffer, (void *) &tamanio_Mens, 4);
	memcpy(buffer + 4, mensaje, tamanio_Mens * sizeof(char));
	int r = send(socket_cliente, buffer, 4 + tamanio_Mens * sizeof(char), 0);
	if (r < 0)
		log_error(logger, "No se pudo enviar el mensaje :c");
	else {
		log_info(logger, "Se envio el mensaje");
	}
	free(buffer);
}

void recibirMensaje(int socket_cliente, t_log *logger, char *mensaje) {
	size_t tamanioMens;
	int bytes_recibidos = recv(socket_cliente, &tamanioMens, 4, 0);
	if (bytes_recibidos < 0) {

		log_error(logger, "No se pudo recibir el mensaje :c");
		exit(1);
	}
	mensaje = malloc(tamanioMens);
	bytes_recibidos = recv(socket_cliente, mensaje, tamanioMens, 0);
	log_info(logger, "Se pudo recibir el mensaje");

}

int esperandoUnaConexion(int socket_servidor, t_log *logger, int numConexion) {
	struct sockaddr cliente_tam;
	unsigned int sin_size = sizeof(struct sockaddr_in);
	int cliente = accept(socket_servidor, &cliente_tam, &sin_size);
	if (cliente < 0) {
		log_info(logger, "ERROR No se pudo aceptar la conexion %d",
				numConexion);
		exit(1);
	}
	log_info(logger, "Se acepto la conexion %d", numConexion);
	return cliente;
}
int connect_to_server(char * ip, char * port, t_log *logger) {
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, port, &hints, &server_info);
	int cliente_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);
	int res = connect(cliente_socket, server_info->ai_addr,
			server_info->ai_addrlen);

	freeaddrinfo(server_info);
	if (res < 0) {
		perror("No se pudo conectar al servidor");
		exit(1);
	}
	log_info(logger, "Conectado!");
	return cliente_socket;
}

int crear_server(char * ip, char * port, t_log *logger) {
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int a = getaddrinfo(ip, port, &hints, &server_info);
	if (a < 0) {
		perror("Error");
		log_error(logger, "No se pudo usar la estructura");
		exit(1);
	}
	int server_socket = socket(server_info->ai_family, server_info->ai_socktype,
			server_info->ai_protocol);
	int res = bind(server_socket, server_info->ai_addr,
			server_info->ai_addrlen);
	freeaddrinfo(server_info);
	if (res < 0) {
		perror("No se pudo bindear");
		exit(1);
		//_exit_with_error(server_socket, "No se puedo bindear", NULL);
	}
	log_info(logger, "Bindeado!");
	int activado = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));
	listen(server_socket, 100);
	return server_socket;
}

void sePudoEnviar(t_log*logger, int conexion, char* mensaje) {
	if (conexion > 0) {
		log_info(logger, "el mensaje: %s \n se pudo enviar", mensaje);
	} else {
		log_error(logger, "el mensaje:%s \n no fue enviado :(", mensaje);
	}
}

int getHeaderSendHandshake(int deQuien, t_log*logger) {

	void* bufferHeader = malloc(4+4);
	int receiver = recv(deQuien, bufferHeader, 8, MSG_WAITALL);
	recepcionMensaje(receiver, logger,"el encabezado\n");
/*
	int handShake = send(deQuien, "1", 2, 0);
	sePudoEnviar(logger, handShake, "1");
*/
	int tamanioPayload;
	memcpy(&tamanioPayload,bufferHeader+4,4);

	free(bufferHeader);
	return tamanioPayload;
}
int getHeaderNoBloqueante(int deQuien, t_log*logger) {

	void* bufferHeader = malloc(4+4);
	int receiver = recv(deQuien, bufferHeader, 8,0);

	if(receiver>0){
		int tamanioPayload;
			memcpy(&tamanioPayload,bufferHeader+4,4);
			free(bufferHeader);

			return tamanioPayload;
		}

	log_info(logger,"de la direccion:%d no se recibio nada",deQuien);
	return receiver;
//recepcionMensaje(receiver, logger,"el encabezado\n");
/*
	int handShake = send(deQuien, "1", 2, 0);
	sePudoEnviar(logger, handShake, "1");
*/
}

void getPayload(void*payload, int tamanioPayload, int deQuien, t_log*logger) {

	int recibirPayload = recv(deQuien, payload, tamanioPayload, MSG_WAITALL);
	recepcionMensaje(recibirPayload, logger,"payload recibido\n");

}

void enviarHeaderConHandshake(header*head, int aQuien, t_log*logger) {


	void*encabezado = malloc(4+4);
	memcpy(encabezado,&head->id,4);
	memcpy(encabezado+4,&head->sizePayload,4);


	int enviar = send(aQuien, encabezado,8, 0);
	sePudoEnviar(logger, enviar,"encabezado");

	/*char*buffer = malloc(2);
	int recibirHandshake = recv(aQuien, buffer, 2, MSG_WAITALL); //espero un 1\0 de respuesta
	recepcionMensaje(recibirHandshake, logger, buffer);

	free(buffer);*/
	free(encabezado);
}
int tamanioTotalPaths(t_list*listaDirecciones){
	int tamanio=0;
		for (int i=0;i<list_size(listaDirecciones);i++){
			int tamanioDespreciable=0;
			memcpy(&tamanioDespreciable,list_get(listaDirecciones,i)+4,4);
			tamanio+=tamanioDespreciable;
		}
	return tamanio;
}

void* dtbAPaquete(DTB*dtb){ //se libera afuera
	int tamPath=strlen(dtb->path)+1;
	int tamLista=list_size(dtb->tablaDeDirecciones);

	//8*tamLista porque va a haber 2 ints por integrante de la lista, el tamanio total de paths son
	//la cantidad total de bytes que ocupan todos los path juntos
	//a eso le sumo todos los /0
	void*paquete = malloc(32+4+4+tamPath+4+8*tamLista+tamanioTotalPaths(dtb->tablaDeDirecciones));
	memcpy(paquete,&dtb->ID,4);
	memcpy(paquete+4,&dtb->FlagInic,4);
	memcpy(paquete+8,&dtb->estado,4);
	memcpy(paquete+12,&dtb->PC,4);
	memcpy(paquete+16,&dtb->quantum,4);
	memcpy(paquete+20,&dtb->memoryPointerEscriptorio,4);
	memcpy(paquete+24,&dtb->IO_Bound,4);
	memcpy(paquete+28,&dtb->IO_Exec,4);
	memcpy(paquete+28+4,&dtb->sentNEW,4);
	memcpy(paquete+28+4+4,&tamPath,4);
	memcpy(paquete+32+4+4,dtb->path,tamPath);
	memcpy(paquete+32+4+4+tamPath,&tamLista,4);
	int listaEnPaq=32+4+4+tamPath+4;//despues de este numero hay un elemento de la lista
	for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
		int tamPathLista=0;
		memcpy(&tamPathLista,list_get(dtb->tablaDeDirecciones,i)+4,4);
		memcpy(paquete+listaEnPaq,list_get(dtb->tablaDeDirecciones,i),8+tamPathLista);
		listaEnPaq+=8+tamPathLista;
	}
	return paquete;
}
void enviarDTB(DTB*dtb, int aQuien, t_log*logger) {

	int tamPath=strlen(dtb->path)+1;
	int tamLista=list_size(dtb->tablaDeDirecciones);
	void*paquete = dtbAPaquete(dtb);

	//envio de header
	int bytesEnvio=32+4+4+tamPath+4+8*tamLista+tamanioTotalPaths(dtb->tablaDeDirecciones);

	header* head = malloc(sizeof(header));
	head->id = 1; // por ejemplo

	head->sizePayload = bytesEnvio;
	log_info(logger,"el paquete pesa %d bytes \n",head->sizePayload);

	enviarHeaderConHandshake(head, aQuien, logger);
	free(head);
	log_info(logger, "Se envia el paquete\n");

	int enviar = send(aQuien,paquete,bytesEnvio, 0);//agregar aca tambien
	sePudoEnviar(logger, enviar,"ok");
	free(paquete);
}
void informarDTB(DTB*dtb,t_log*logger){
	log_info(logger,"id:%d\n",dtb->ID);
	log_info(logger,"flag:%d\n",dtb->FlagInic);
	log_info(logger,"estado:%d\n",dtb->estado);
	log_info(logger,"PC:%d\n",dtb->PC);
	log_info(logger,"Quantum:%d\n",dtb->quantum);
	log_info(logger,"Puntero a memoria:%d\n",dtb->memoryPointerEscriptorio);
	log_info(logger,"operaciones IO:%d\n",dtb->IO_Bound);
	log_info(logger,"Path:%s\n",dtb->path);
	log_info(logger,"tabla de direcciones:\n");

	for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
		int memoria=0;
		memcpy(&memoria,list_get(dtb->tablaDeDirecciones,i),4);
		int tamPath=0;
		memcpy(&tamPath,list_get(dtb->tablaDeDirecciones,i)+4,4);
		char*path=malloc(tamPath);
		memcpy(path,list_get(dtb->tablaDeDirecciones,i)+8,tamPath);
		log_info(logger,"posicion de la lista %d",i);
		log_info(logger,"espacio en memoria:%d",memoria);
		log_info(logger,"path:%s\n",path);
		free(path);
	}
}
void cargarDTBSinInformar(DTB*dtb, void* paquete, t_log*logger) {
	int tamPath;
	memcpy(&dtb->ID,paquete,4);
	memcpy(&dtb->FlagInic,paquete+4,4);
	memcpy(&dtb->estado,paquete+8,4);
	memcpy(&dtb->PC,paquete+12,4);
	memcpy(&dtb->quantum,paquete+16,4);
	memcpy(&dtb->memoryPointerEscriptorio,paquete+20,4);
	memcpy(&dtb->IO_Bound,paquete+24,4);
	memcpy(&dtb->IO_Exec,paquete+28,4);
	memcpy(&dtb->sentNEW,paquete+28+4,4);
	memcpy(&tamPath,paquete+28+4+4,4);
	dtb->path=malloc(tamPath); //esta memoria se libera afuera
	memcpy(dtb->path,paquete+32+4+4,tamPath); //ajustar los bytes necesarios
	//cargar la lista
	dtb->tablaDeDirecciones=list_create();
	int tamLista=0;
	memcpy(&tamLista,paquete+32+4+4+tamPath,4);
	int punteroParteLista=32+4+4+tamPath+4;
	for(int i=0;i<tamLista;i++){
		int tamPathLista=0;
		memcpy(&tamPathLista,paquete+punteroParteLista+4,4);//apunta al tamaño del path de la parte
		void*paqueteElemLista=malloc(8+tamPathLista);
		memcpy(paqueteElemLista,paquete+punteroParteLista,8+tamPathLista);
		list_add(dtb->tablaDeDirecciones,paqueteElemLista);
		punteroParteLista+=8+tamPathLista;
	}
}
void cargarDTB(DTB*dtb, void* paquete, t_log*logger) {

	cargarDTBSinInformar(dtb,paquete,logger);
	informarDTB(dtb,logger);
}

int dameMaximo(int *tabla, int n) {
	int i;
	int max;

	if ((tabla == NULL) || (n < 1))
		return 0;

	max = tabla[0];
	for (i = 0; i < n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}

void nuevoCliente(int servidor, int *clientes, int *nClientes, t_log *logger) {
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = esperandoUnaConexion(servidor, logger, 100);
	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */
	if ((*nClientes) >= MAX_CLIENTES) {
		close(clientes[(*nClientes) - 1]);
		(*nClientes)--;
		return;
	}

	/* Envía su número de cliente al cliente */
//	Escribe_Socket (clientes[(*nClientes)-1], (char *)nClientes, sizeof(int));
	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	printf("Aceptado cliente %d\n", *nClientes);
	return;
}
void nuevaCPU(int servidor, CPU clientes[], int *nClientes, t_log *logger) {
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes].conexion = esperandoUnaConexion(servidor, logger, 100);

	clientes[*nClientes].disponibilidad=1;
	char*buffer=malloc(4);
	recv(clientes[*nClientes].conexion,buffer,4,MSG_WAITALL);
	free(buffer);
	(*nClientes)++; //agregar las cosas a la bolsa

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */
	if ((*nClientes) >= MAX_CLIENTES) {
		close(clientes[(*nClientes) - 1].conexion);
		(*nClientes)--;
		return;
	}

	/* Envía su número de cliente al cliente */
//	Escribe_Socket (clientes[(*nClientes)-1], (char *)nClientes, sizeof(int));
	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	printf("Aceptado cliente %d\n", *nClientes);
	return;
}
void compactaClaves (int *tabla, int *n)
{
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}
void mandarRutinaAlSafa(diego_safa*rutina,int conexionSAFA,t_log*logger){
	int tamPath=strlen(rutina->path)+1;
	int paqueteAEnviar=16+tamPath;
	void*paquete=malloc(paqueteAEnviar);
		memcpy(paquete,&rutina->tipo,4);
		memcpy(paquete+4,&rutina->id,4);
		memcpy(paquete+8,&rutina->memoryPointer,4);
		memcpy(paquete+12,&tamPath,4);
		memcpy(paquete+16,rutina->path,tamPath);
		header*head=malloc(sizeof(header));
		head->id=1;
		head->sizePayload=paqueteAEnviar;
		enviarHeaderConHandshake(head,conexionSAFA,logger);
		send(conexionSAFA,paquete,paqueteAEnviar,0);
		log_info(logger,"se mando una rutina de tipo %d al safa",rutina->tipo);
	free(head);
	free(paquete);
}
void recibirRutinaDelDiego(diego_safa*rutina,int conexionDiego,t_log*logger){
	int size=getHeaderSendHandshake(conexionDiego,logger);
	void*paquete=malloc(size);
	getPayload(paquete,size,conexionDiego,logger);
		memcpy(&rutina->tipo,paquete,4);
		memcpy(&rutina->id,paquete+4,4);
		memcpy(&rutina->memoryPointer,paquete+8,4);
		int tamPath=0;
		memcpy(&tamPath,paquete+12,4);

		rutina->path=malloc(tamPath);
		memcpy(rutina->path,paquete+16,tamPath);

	free(paquete);
}

void enviarString(char* string, int aQuien, t_log*logger) {

	int tamString=strlen(string);
	void*paquete = malloc(4+tamString+1);
	memcpy(paquete,&tamString,4);
	memcpy(paquete+4,string,tamString+1);

	//envio de header

	header* head = malloc(sizeof(header));
	head->id = 1; // por ejemplo
	head->sizePayload = 4+tamString+1;

	log_info(logger,"el paquete pesa %d bytes \n",head->sizePayload);

	enviarHeaderConHandshake(head, aQuien, logger);
	free(head);
	log_info(logger, "Se envia el paquete\n");

	int enviar = send(aQuien,paquete,4+tamString+1, 0);
	sePudoEnviar(logger, enviar,"ok");
	free(paquete);
}


char* cargarString(void* paquete, t_log*logger) {

	int tamString;
	memcpy(&tamString,paquete,4);


	log_info(logger, "el tamanio del string es: %d + /0\n",tamString);
	char*string=malloc(tamString+1); //esta memoria se libera afuera
	memcpy(string,paquete+4,tamString+1); //ajustar los bytes necesarios

	log_info(logger,"String:%s\n",string);
	return string;
}

void esperarMili(int tiempo)
{
    struct timespec ts;
    ts.tv_sec = tiempo / 1000;
    ts.tv_nsec = (tiempo % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void enviarIntconString(char* string,int num, int aQuien, t_log*logger) {

	int tamString=strlen(string);
	void*paquete = malloc(4+tamString+1);
	memcpy(paquete,&tamString,4);
	memcpy(paquete+4,&num,4);
	memcpy(paquete+8,string,tamString+1);

	//envio de header

	header* head = malloc(sizeof(header));
	head->id = 1; // por ejemplo
	head->sizePayload = 4+4+tamString+1;

	log_info(logger,"el paquete pesa %d bytes \n",head->sizePayload);

	enviarHeaderConHandshake(head, aQuien, logger);
	free(head);
	log_info(logger, "Se envia el paquete\n");

	int enviar = send(aQuien,paquete,4+4+tamString+1, 0);
	sePudoEnviar(logger, enviar,"ok");
	free(paquete);
}
void liberar_lista(t_list*lista){
	for(int i=list_size(lista)-1;i>=0;i--){
		list_remove_and_destroy_element(lista,i,(void*) liberarDTBLista);
	}
}
void liberarDTBLista(void*paquete){
	free(paquete);
}


