/*
 * lector.c
 * este programa va a leer los archivos de escriptorio
 */
#include "gs.h"
#include"protocolos.h"
#include "FuncionesEscriptorio.h"

extern t_log* logger;
extern t_log* loggerError;

enum errores {
	SINTAX_ERROR,
	E10001,
	E10002,
	E20001,
	E20002,
	E20003,
	E30001,
	E30002,
	E30003,
	E30004,
	E40001,
	E40002,
	E50001,
	E50002,
	E60001
};

//14 errores

char *palabrasReservadas[9] = { "abrir", "concentrar", "asignar", "wait",
		"signal", "close", "flush", "crear", "borrar" };

char *tipoPalabra(char *);
int ejecutarLinea(char *, char *,DTB*);
int huboError(int,char*);

int ejecutar(char* linea,DTB*dtb) {

	char**comandoALiberar=string_n_split(linea,2, " ");
	char* comando =comandoALiberar[0]; //Se fija cual es la funcion a ejecutar
	//printf("COMANDO QUE VAMO A EJECUTAR: %s",comando);
	log_info(logger,"accion:%s",comando);
	char** parametrosALiberar=string_n_split(linea,2, " ");
	char* parametros = parametrosALiberar[1]; //Se fija con que parametro
	int resultado;


	log_info(logger,"parametros:%s",parametros);
	resultado = ejecutarLinea(comando, parametros,dtb); //Ejecuta la linea

	resultado = huboError(resultado,linea);
	free(comandoALiberar[0]);
	free(comandoALiberar[1]);
	free(comandoALiberar);
	free(parametrosALiberar[0]);
	free(parametrosALiberar[1]);
	free(parametrosALiberar);
	//free(comando);
	//free(parametros);

	return resultado;
}

//---------------------------------------------------------------------
int huboError(int ejecucion,char* linea) {

	switch (ejecucion) {
	case SINTAX_ERROR:
		log_info(loggerError, "ERROR SINTACTICO en %s",linea);
		return -1;
	case E10001:
		log_info(loggerError, "ERROR 10001");
		return -1;
	case E10002:
		log_info(loggerError, "ERROR 10002");
		return -1;
	case E20001:
		log_info(loggerError, "ERROR 20001");
		return -1;
	case E20002:
		log_info(loggerError, "ERROR 20002");
		return -1;
	case E20003:
		log_info(loggerError, "ERROR 20003");
		return -1;
	case E30001:
		log_info(loggerError, "ERROR 30001");
		return -1;
	case E30002:
		log_info(loggerError, "ERROR 30002");
		return -1;
	case E30003:
		log_info(loggerError, "ERROR 30003");
		return -1;
	case E30004:
		log_info(loggerError, "ERROR 30004");
		return -1;
	case E40001:
		log_info(loggerError, "ERROR 40001");
		return -1;
	case E40002:
		log_info(loggerError, "ERROR 40002");
		return -1;
	case E50001:
		log_info(loggerError, "ERROR 50001");
		return -1;
	case E50002:
		log_info(loggerError, "ERROR 50002");
		return -1;
	case E60001:
		log_info(loggerError, "ERROR 60001");
		return -1;
	default:
		return ejecucion;
	}
}

//---------------------------------------------------------------------

char *tipoPalabra(char *linea) {

	char* palabra = (char*) string_split(linea, " ")[0];

	for (int i = 0; i != 10; i++) //Checkea si la palabra es una reservada
			{
		if (palabra == palabrasReservadas[i]) {
			return palabra;
		}
	}

	return "SYNTAX ERROR"; //Si no es una reservada, devuelve error

}

//---------------------------------------------------------------------

int ejecutarLinea(char*palabra, char*parametro,DTB*dtb) {

// IMPORTANTE 100 significa ok en cada caso
	int resultado;

	if (string_equals_ignore_case(palabra, palabrasReservadas[0])) { //ABRIR
		//printf("EJECUTANDO ABRIR: %s",palabra);
		resultado = abrir(parametro,dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[1])) { //CONCENTRAR
		//Operacion nula
		//sleep(1);
		return 100;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[2])) { //ASIGNAR

		resultado = asignar(parametro,dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[3])) { //WAIT

		resultado = wait(parametro,dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[4])) { //SIGNAL

		resultado = signal(parametro,dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[5])) { //CLOSE

		resultado = closeTP(parametro,dtb); //Mayuscula porque si no se confunde con close() (funcion de c)
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[6])) { //FLUSH

		resultado = flush(parametro, dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[7])) { //CREAR

		resultado = crear(parametro,dtb);
		return resultado;
	}
	if (string_equals_ignore_case(palabra, palabrasReservadas[8])) { //BORRAR

		resultado = borrar(parametro,dtb);
		return resultado;
	}
	return 0;			//error sintactico

}

//---------------------------------------------------------------------
