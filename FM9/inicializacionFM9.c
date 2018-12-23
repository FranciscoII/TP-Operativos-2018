#include "gs.h"
#include "funcionesFM9.h"

#define TAM_MAX_SEG 3 //3 lineas tiene el segmento
#define TAM_MAX_LINEA 32
#define STORAGE 1024
#define TAM_PAG 64

extern t_list* tablaSegmentos;
extern t_list* tablaInvertida;
extern t_list* TI_tablaInfoAuxilar;
extern int tamanioTabla;

void inicializarSegmentacion() {
	tablaSegmentos = list_create();
	return;
}

void inicializarTablaInvertida() {
	tablaInvertida = list_create();
	TI_tablaInfoAuxilar = list_create();

	t_TablaINV* elemento;
	t_TI_InfoAux* auxElemento;



	for(int i = 0; i < tamanioTabla; i++) //Se crean todos los elementos de la tabla que pueden haber
	{
		elemento = malloc(sizeof(t_TablaINV));
		elemento->PID = -1; //-1 significa que no esta asignado
		elemento->pagina = -1; //Ditto
		elemento->indice = i; //Asignacion de marco
		list_add(tablaInvertida,elemento);

		auxElemento = malloc(sizeof(t_TI_InfoAux));
		auxElemento->marco = i;
		auxElemento->lineasOcupadas = 0;
		auxElemento->terminaUnArchivo = 0;
		list_add(TI_tablaInfoAuxilar,auxElemento);
	}

	return;
}

void inicializarSegmentacionPaginada(){
	tablaSegmentos = list_create();
	return;
}

void agregar(char* memoria) { //es solo para pruebas
//	FILE* archivo;
//	archivo = fopen("data.txt","r"); //simula que le pasa lo de socket (3 lineas)
	char* linea = malloc(TAM_MAX_LINEA);
	int cantidadLineas = 3;
	int base;
	verArrayBits();
	if ((base = obtenerBaseParaSegDeNLineas(cantidadLineas)) != -1) {
		printf("base: %d\n", base);

	}
}

/*		while(fgets(linea, TAM_MAX_LINEA, archivo)) {
 if(!(string_equals_ignore_case(linea,"\n"))){//ignora las lineas vacias
 memcpy(memoria+((base+cantidadLineas)*TAM_MAX_LINEA),linea,TAM_MAX_LINEA);
 setear(base+cantidadLineas);
 cantidadLineas++;
 printf("Hola amigos");
 }
 }*/
