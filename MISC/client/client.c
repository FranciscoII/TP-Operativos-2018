/*
 * client.c
 *
 *  Created on: 4 sep. 2018
 *      Author: utnso
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
struct sockaddr_in direccionServidor;
direccionServidor.sin_family= AF_INET;
direccionServidor.sin_addr.s_addr=inet_addr("127.0.0.1");
direccionServidor.sin_port=htons(9001);

int cliente= socket(AF_INET,SOCK_STREAM,0);

if(connect(cliente,(struct sockaddr *)&direccionServidor,sizeof(direccionServidor))!=0){
	perror("no se pudo conectar");
	return 1;
}

while(1){
char* buffer= malloc(100);
scanf("%s",buffer);
send(cliente,buffer,strlen(buffer),0);
free(buffer);
}


return 0;
}











