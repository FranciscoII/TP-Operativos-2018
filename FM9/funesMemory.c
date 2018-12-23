#include "gs.h"
#include "funcionesFM9.h"

t_list* tablaSegmentos;
t_list* tablaInvertida;
t_list* TI_tablaInfoAuxilar;
t_config*config;
t_log* loggerFM9;
sem_t sem_guardarMemoria;
sem_t mutexTabla;
sem_t cerrarFM9;
char* storage;
int tamanioTabla;
int tam_max_linea;
int tam_max_pag;
int tamanioMemoria;

void agregar(char* memoria);

int main() {
	//printf("\n\x1b[31m");//color fm9

	loggerFM9 = log_create("FM9.log", "FM9", 1, 0);
//	tablaSegmentos = list_create();
	sem_init(&mutexTabla, 0, 1);
	sem_init(&cerrarFM9, 0, 0);
	config =
			config_create(
					"/home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/CONFIG/FM9.txt");
	tamanioMemoria = config_get_int_value(config, "TAMANIO");
	storage = malloc(tamanioMemoria); //inicializa la memoria
	tam_max_pag = config_get_int_value(config, "TAM_PAGINA");
	tam_max_linea = config_get_int_value(config, "MAX_LINEA");
	tamanioTabla = (config_get_int_value(config, "TAMANIO")) / tam_max_pag;

	crear_Archivo_Bitmap();
	switch (devolverModo(config)) {
	case SEG:
		inicializarSegmentacion();
		printf("Elegiste segmentacion pura joven Padawan\n");
		break;
	case PAG:
		inicializarTablaInvertida();
		printf("Elegiste paginacion invertida lince\n");
		break;
	case SEGPAG:
		inicializarSegmentacion();
		printf("Elegiste segmentacion paginada papu\n");
		break;
	default:
		break;

	}
	pthread_t comunicaciones; //hilo comunicaciones
	pthread_t h_consola; //hilo consola
	pthread_create(&comunicaciones, NULL, (void*) coms, NULL);
	pthread_create(&h_consola, NULL, (void*) consola, NULL);
	pthread_detach(comunicaciones);
	/*
	 //agrego un segmento con 3 lineas a modo de ejemplo
	 agregar(storage);
	 */

//	pthread_join(consola,NULL);
	sem_wait(&cerrarFM9);
	//pruebas para la lista..
	/*
	 int basePrueba = encontrarBaseSegmento(2);
	 int offsetPrueba = 0;
	 int limitePrueba = encontrarLimiteSegmento(2);
	 if(basePrueba == -1 || limitePrueba<offsetPrueba){
	 printf("NO existe ese segmento amigazo o hiciste ALTO segFAULT");
	 }
	 else{
	 char*lineaPrueba = malloc(TAM_MAX_LINEA);
	 memcpy(lineaPrueba,storage+(basePrueba+offsetPrueba)*32,32);
	 printf("los datos del segmento ID:2 son: %s\n");
	 }*/
	free(storage);
	switch (devolverModo(config)) { //hay que destruir los elementos tambien.
	case SEG:
		liberar_tablaSegmentos(tablaSegmentos);
		list_destroy(tablaSegmentos);
		break;
	case PAG:

		liberar_tablaInvertida(tablaInvertida, TI_tablaInfoAuxilar,
				tamanioTabla);
		list_destroy(tablaInvertida);
		list_destroy(TI_tablaInfoAuxilar);
		break;
	case SEGPAG:
		liberar_tablaSegmentosPag(tablaSegmentos);
		list_destroy(tablaSegmentos);
		break;
	}
	config_destroy(config);
	log_destroy(loggerFM9);
	sem_destroy(&cerrarFM9);
	return 0;
}

/*
 //	printf("La cantidad Maxima de Lineas sera : %d \n",cantidadMaximaLineas);
 //
 //	for(int i=0;i<cantidadMaximaLineas-1;i++)
 //	{
 //		memcpy(sto+(i*TAM_MAX_LINEA),"\n",TAM_MAX_LINEA);
 //	}
 //	memcpy(sto+(31*TAM_MAX_LINEA),"Hola capo\n",TAM_MAX_LINEA);

 crear_Archivo_Bitmap();
 int* lineas;// = obtenerIdBloquesDisponibles(4);
 //free(lineas);



 int auxBase = 0;



 verArrayBits();
 printf("el storage tiene linea 1: %s",sto);
 printf("el storage tiene linea 2: %s",sto+(TAM_MAX_LINEA));
 printf("el storage tiene linea 3: %s",sto+(2*TAM_MAX_LINEA));

 //calcula cuanto lugar ocupa el "SEGMENTO"
 t_segmentacion *segmento1 =malloc(sizeof(t_segmentacion));
 segmento1->id = 1;
 segmento1->limite = cantidadLineas * TAM_MAX_LINEA;//esto seria cuantos bytes ocupa nuestro segmento
 segmento1->base = auxBase;

 //Ahora le pido que me tire la linea 4 del segmento 1
 int numLin = 3;
 char newLine[TAM_MAX_LINEA];
 memcpy(newLine,sto+(numLin*TAM_MAX_LINEA),TAM_MAX_LINEA);
 printf("La linea 4 es(obtenida desde el storage): %s",newLine);

 //PRUEBA NQV seteo en 1 el bit 8
 int prueb = setear(9);
 verArrayBits();


 //------Imaginemos que tenemos un segmento de 3 lineas y queremos usar FIRST-FIT para ponerlo en mem-------------

 int lineasRecibidas = 7;
 char* Repiola = "esta linea esta bien pilla";
 //	int baseSeg = obtenerBaseParaSegDeNLineas(lineasSegmento);//YA FUNCIONA!!!
 //	printf("Tu base es %d",baseSeg);
 //	//ahora en esa base empezamos a darle masa y meter las lineas
 //	//for(int k=0;k<3;k++){//escribimos las N lineas que sean
 //		memcpy(sto+(baseSeg)*TAM_MAX_LINEA,Repiola,TAM_MAX_LINEA);
 //	//}
 //prueba de que divide segmentos. como el tamaño max del segmento es 3 va a dividir por la mitad
 if(lineasRecibidas > TAM_MAX_SEG){
 int NumeroDeSegmentos = lineasRecibidas/TAM_MAX_SEG;
 //printf("cantidad: %d",NumeroDeSegmentos);
 int resto = 0;
 if(lineasRecibidas%TAM_MAX_SEG){
 NumeroDeSegmentos += 1;
 resto = lineasRecibidas%TAM_MAX_SEG;
 }
 //int* segmentos = malloc(NumeroDeSegmentos*sizeof(int));
 int baseSeg;
 for(int q=0;q<NumeroDeSegmentos;q++){
 if(lineasRecibidas == resto){
 baseSeg = obtenerBaseParaSegDeNLineas(resto);
 if(baseSeg == -1)
 return -1;//no hay espacio!
 //printf("base %d: %d\n",q+1,baseSeg);
 }
 else{
 baseSeg = obtenerBaseParaSegDeNLineas(TAM_MAX_SEG);lineasRecibidas-=3;
 if(baseSeg == -1)
 printf("No hay space");//no hay espacio!
 }
 printf("base %d: %d\n",q+1,baseSeg);
 setear(baseSeg);//Igual deberia setear todos los bit que ocupa a 1..
 //setear(baseSeg+2);
 }
 //free(segmentos);
 }
 else{
 int basecita = obtenerBaseParaSegDeNLineas(lineasRecibidas);
 printf("base Unica: %d",basecita);
 if(basecita == -1)
 return -1;//no hay espacio!
 }


 verArrayBits();

 free(lineas);
 fclose(archivo);
 free(sto);
 free(segmento1);
 log_destroy(loggerFM9);
 return 0;
 }
 */
