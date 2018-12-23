#include "conexionInicial.h"

int sePudieronConectar(int conexion1, int conexion2, t_log*logger) {
	if (conexion1 > 0 && conexion2 > 0) {
		log_info(logger, "ambas conexiones se realizaron");
		return 1;
	}
	log_error(logger, "alguna conexion fallo");
	return 0;
}

int sonElDamYunCpu(char* bufferId1, char*bufferId2, t_log*logger) {
	if (string_equals_ignore_case(bufferId1, "DAM")
			&& string_equals_ignore_case(bufferId2, "CPU")) {
		log_info(logger, "la conexion 1 es diego");
		return 1;
	}
	if (string_equals_ignore_case(bufferId2, "DAM")
			&& string_equals_ignore_case(bufferId1, "CPU")) {
		log_info(logger, "la conexion 2 es diego");
		return 2;
	}
	return 0;

}

int seConectaronElDamYUnCpu(int conexion1, int conexion2, char*bufferId1,
		char*bufferId2, t_log*logger) {
	if (sePudieronConectar(conexion1, conexion2, logger)) {
		int diego = sonElDamYunCpu(bufferId1, bufferId2, logger);
		return diego;
	} else {
		return 0;
	}
}

int soyElDiego(int validacion, int conexion1, int conexion2) {
	if (validacion == conexion1) {
		return conexion1;
	} else {
		return conexion2;
	}
}

int soyElCpu(int validacion, int conexion1, int conexion2) {
	if (validacion != conexion1) {
		return conexion1;
	} else {
		return conexion2;
	}
}

