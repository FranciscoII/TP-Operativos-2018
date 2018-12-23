#include "operacionesDAM.h"
#include "gs.h"

extern int diego;
extern int safa;
extern t_log *logger;
extern int fm9;
void finalizar(){
	int accion = 3;
	int tamDTBDelSafa=getHeaderSendHandshake(safa,logger);
	void*paquete=malloc(tamDTBDelSafa);
	DTB*dtb=malloc(tamDTBDelSafa);
	cargarDTB(dtb,paquete,logger);
	//es por cada archivo
	void* panela= malloc(12);

	memcpy(panela,&accion,4);
	memcpy(panela+4,&dtb->memoryPointerEscriptorio,4);
	memcpy(panela+8,&dtb->ID,4);

	send(fm9,&diego,4,0);
	send(fm9,panela,12,0);

	for(int i=0;i<list_size(dtb->tablaDeDirecciones);i++){
		memcpy(panela+4,list_get(dtb->tablaDeDirecciones,i),4);
		send(fm9,&diego,4,0);
		send(fm9,panela,12,0);
	}
	free(panela);
}

