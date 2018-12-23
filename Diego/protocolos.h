/*
 * protocolos.h
 *
 *  Created on: 20 sep. 2018
 *      Author: utnso
 */

#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

#include<sys/types.h>
typedef enum{
	VALIDARARCHIVO,
	BORRARARCHIVO,
	CREARARCHIVO,
	OBTENERDATOS,
	GUARDARDATOS

}operacionMDJ;

typedef struct{
	size_t tamanioPath;
	operacionMDJ cod;
	char*path;
	off_t offset;
	size_t tam;
	void *buf;
	int nbytes;
}instruccionaMDJ;
typedef enum{
	TODOOKEY,
	DATAOBTENIDA,
	TIPOERROR
}rtaaDIEGO;

typedef struct{
	rtaaDIEGO rta;
	void* buffer;
	size_t tamBuffer;
	int error;
}devMDJ;
#endif /* PROTOCOLOS_H_ */

