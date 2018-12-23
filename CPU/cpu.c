#include "gs.h"
#include"protocolos.h"
#include"FuncionesEscriptorio.h"
#include "lector.h"

//-------
t_log* logger;
t_log* loggerError;
int diego;
t_config* config;
int fm9;
int safa;

int ejecutar();
int main() {

	printf("\n\x1b[36mCPU lista para ejecutar!\x1b[0m\n");



	config =
			config_create(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/CPU.txt");

	int resultado=0;
	int dummy = 1;
	logger = log_create("cpu.log", "cpu", 0, 0);
	loggerError = log_create("cpuError.log", "cpu", 0, 0);
	int pedirLinea = 1;
	safa = connect_to_server(config_get_string_value(config, "IP_SAFA"),
			config_get_string_value(config, "PUERTO_SAFA"), logger);
	fm9 = connect_to_server(config_get_string_value(config, "IP_MEM"),
				config_get_string_value(config, "PUERTO_MEM"), logger);

	void*soycpu = malloc(4);
	int cpu = 1;
	memcpy(soycpu, &cpu, 4);
	send(safa, soycpu, 4, 0);
	send(fm9,soycpu,4,0);
	free(soycpu);
	diego = connect_to_server(config_get_string_value(config, "IP_DIEGO"),
			config_get_string_value(config, "PUERTO_DIEGO"), logger);


	while (1) {
		int tamanioPayload = getHeaderSendHandshake(safa, logger);
		void *dtbEnVoid = malloc(tamanioPayload);
		getPayload(dtbEnVoid, tamanioPayload, safa, logger);

		//printf("\n\x1b[36mSe recibio el siguiente DTB:\x1b[0m\n");

		DTB*nuevoDTB = malloc(sizeof(DTB));
		cargarDTB(nuevoDTB, dtbEnVoid, logger);
			printf("\n\x1b[36mProceso %d a ejecutar\x1b[0m\n",nuevoDTB->ID);




		//esperarMili(config_get_int_value(config, "PUERTO")); //Retardo de ejecucion


		if (nuevoDTB->FlagInic == 0) { //Es el Dummy
			log_info(logger, "El DTB es el Dummy");
			send(diego, &dummy, 4, 0); //Manda 1 si es el dummy
			enviarDTB(nuevoDTB, diego, logger);

		} else if(nuevoDTB->FlagInic == 2){
			closeEscriptorio(nuevoDTB);
			for(int i=list_size(nuevoDTB->tablaDeDirecciones)-1;i>=0;i--){
				int tamanioPath=0;
				memcpy(&tamanioPath,list_get(nuevoDTB->tablaDeDirecciones,i)+4,4);
				char*pathACerrar=malloc(tamanioPath);
				memcpy(pathACerrar,list_get(nuevoDTB->tablaDeDirecciones,i)+8,tamanioPath);
				closeTP(pathACerrar,nuevoDTB);
				free(pathACerrar);
			}
			//printf("\n\x1b[36mSe va a liberar la memoria del proceso %d!\x1b[0m\n",nuevoDTB->ID);
		}else{ //Ejecutar las lineas


			int quantumSAFA = nuevoDTB->quantum;
			while (quantumSAFA != 0) {

			memcpy(&soycpu, &cpu, 4);
			send(fm9,&soycpu,4,0);
			void* paquete = malloc(16);//(base,offset,pid,ACCION) asi se debe cargar
			memcpy(paquete,&nuevoDTB->memoryPointerEscriptorio,4);//base seg
			memcpy(paquete+4,&nuevoDTB->PC,4);//offset
			memcpy(paquete+4+4,&nuevoDTB->ID,4);//PID del proceso  (para paginacion)
			memcpy(paquete+4+4+4,&pedirLinea,4);//pedirLineaAlFM9

			int enviar =send(fm9,paquete,16,0);
			sePudoEnviar(logger,enviar,"ID Y OFFSET");
			free(paquete);

			int tamLinea=0;
			recv(fm9,&tamLinea,4,MSG_WAITALL);
			char* lineaComando = malloc(tamLinea);
			recv(fm9,lineaComando,tamLinea,MSG_WAITALL);
			strtok(lineaComando, "\n");


			printf("\n\x1b[36m\t La linea a ejecutar es: \n\x1b[0m %s\n",lineaComando);

			log_info(logger, "Ejecutando el DTB %d", nuevoDTB->ID);




				if (string_equals_ignore_case(lineaComando,"\n"))
										{ //tom, esto lo cambie de lugar, lo que pasa es que
				nuevoDTB->estado = EXIT; //primero tendria que fijarse si va a ejecutar la linea final.
				free(lineaComando);
				break;					//porque si es la final, el abrir que uso para practicar se seguia ejecutando, osea que le mandaba algo al diego que no debia
								} //seguia ejecutando, osea que le mandaba algo al diego que no debia
								  //(el abrir tiene que mandarle un paquete al diego)


				quantumSAFA--;
				sleep(config_get_int_value(config, "RETARDO")/100);


				resultado = ejecutar(lineaComando,nuevoDTB);

				nuevoDTB->PC++;

				if (resultado==-1)
														{
					nuevoDTB->estado = EXIT;
					resultado=1;
					free(lineaComando);
					break;
							}

				if (resultado == 200) {
					nuevoDTB->quantum=quantumSAFA;
					nuevoDTB->estado = BLOQ;
					free(lineaComando);
					break;
				}
				free(lineaComando);
			}

		}
		printf("\n\x1b[36mDesalojando el proceso %d\x1b[0m\n",nuevoDTB->ID);

		send(safa, &cpu, 4, 0);
		enviarDTB(nuevoDTB, safa, logger);

		liberar_lista(nuevoDTB->tablaDeDirecciones);
		list_destroy(nuevoDTB->tablaDeDirecciones);
		free(dtbEnVoid);
		free(nuevoDTB->path);
		free(nuevoDTB);
	}

	log_destroy(logger);
	config_destroy(config);

	return 0;
}


