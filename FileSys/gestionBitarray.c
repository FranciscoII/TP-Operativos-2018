#include "gestionBitarray.h"
#include "instrucciones.h"
char* camino="/home/utnso/workspace/fifa-entrega/Metadata/Bitmap.bin";
void actualizar_Archivo_Bitmap(){
	char a[TAMBUFFER];
	FILE *fd;
	void* buf[10];
		t_bitarray *bitarray= bitarray_create(a,sizeof(a));
		for(int i=0;i<CANTIDADBLOQUES;i++){
			char* bloq = caminoBloque(i);
			fd = fopen(bloq,"r");
			if(fd == NULL)
				;//setear(i);
			else{
				if(fread(buf,sizeof(char),1,fd) > 0)
					setear(i);
			}
			free(bloq);
		}
		fwrite(a,1,TAMBUFFER,fd);
		//verArrayBits();
		bitarray_destroy(bitarray);
		fclose(fd);

}
void crear_Archivo_Bitmap(){

	printf("%d\n", TAMBUFFER);
	char a[TAMBUFFER];
	t_bitarray *bitarray= bitarray_create(a,sizeof(a));
	for(int i=0;i<CANTIDADBLOQUES;i++)
		bitarray_clean_bit(bitarray,i);
	if((CANTIDADBLOQUES%CHAR_BIT)!=0)
		for(int j=CANTIDADBLOQUES;j<TAMBUFFER*CHAR_BIT;j++){
			bitarray_set_bit(bitarray,j);
		}
	FILE *fd;
	if((fd=fopen(camino,"rb"))==NULL)
		fd=fopen(camino,"wb");
		if(fd<0||fd==NULL){
			printf("Errorrrr r\n");
			exit(1);
		}
		fwrite(a,1,TAMBUFFER,fd);
		//verArrayBits();
		bitarray_destroy(bitarray);
		fclose(fd);


}
char* obtenerDatosBitArray(){
	char *buffer=malloc(TAMBUFFER*	sizeof(char));
	FILE *fd=fopen(camino,"rb");
		if(fd<0||fd==NULL){
				printf("Errorrrr");
				exit(1);
			}
	fread(buffer,TAMBUFFER,TAMBUFFER,fd);
	fclose(fd);
	return buffer;
}
int hayBloquesDisponibles(int cantidad_bloques){
	char *buffer=obtenerDatosBitArray();
	t_bitarray *bitarray=bitarray_create(buffer,strlen(buffer));
	int cantidad=0;
	for(int i=0;(i<CANTIDADBLOQUES)&&(cantidad<cantidad_bloques);i++)
		if(!bitarray_test_bit(bitarray,i))
			cantidad++;
	bitarray_destroy(bitarray);
	free(buffer);
	return (cantidad>=cantidad_bloques);
}
int* obtenerIdBloquesDisponibles(int cantidad_bloques){
	if(!hayBloquesDisponibles(cantidad_bloques))
		return NULL;
	else{
		char *buffer=obtenerDatosBitArray();
		t_bitarray *bitarray=bitarray_create(buffer,strlen(buffer));
		int *a=malloc(cantidad_bloques*sizeof(int));
		int con=0;
		for(int i=0;(i<CANTIDADBLOQUES)&&(cantidad_bloques>0);i++){
			if(!bitarray_test_bit(bitarray,i)){
					a[con]=i;
					cantidad_bloques--;
					con++;
			}
		}
		free(buffer);
		bitarray_destroy(bitarray);

		return a;
	}
}
//SETEAR SE USA DENTRO DE OTRAS FUNCIONES DE ACA, NO COMPUREBA
//SI ESE BLOQUE YA ESTABA USADO
int setear(int idbloque){
	char  *buffer=obtenerDatosBitArray();
	t_bitarray *bitarray=bitarray_create(buffer,strlen(buffer));
	bitarray_set_bit(bitarray,idbloque);
	FILE *fd=fopen(camino,"wb");
			if(fd<0||fd==NULL){
				bitarray_destroy(bitarray);
				free(buffer);
				return -1;
			}
			fwrite(buffer,1,TAMBUFFER,fd);
			free(buffer);
			bitarray_destroy(bitarray);
			fclose(fd);
	return 1;
}
int* reservarBloquesRequeridos(int cantidadBloques){
	int *idBloquesDisponibles=obtenerIdBloquesDisponibles(cantidadBloques);
	if(idBloquesDisponibles==NULL)
		return NULL;
	for(int i=0;i<cantidadBloques;i++){
		if(setear(idBloquesDisponibles[i])<0){
			for(int j=0;j<i;j++)
				desSetear(idBloquesDisponibles[j]);
			return NULL;
		}

	}
	return idBloquesDisponibles;

}



int desSetear(int idbloque){
	char  *buffer=obtenerDatosBitArray();
	t_bitarray *bitarray=bitarray_create(buffer,strlen(buffer));
	bitarray_clean_bit(bitarray,idbloque);
	FILE *fd=fopen(camino,"wb");
			if(fd<0||fd==NULL){
				bitarray_destroy(bitarray);
				free(buffer);
				return -1;
			}
		fwrite(buffer,1,TAMBUFFER,fd);
		free(buffer);
			bitarray_destroy(bitarray);
			fclose(fd);
	return 1;
}
void verArrayBits(){
	char*buffer=obtenerDatosBitArray();
	t_bitarray *bitarray=bitarray_create(buffer,strlen(buffer));
	for(int i=0;i<TAMBUFFER*CHAR_BIT;i++){
			if((i%CHAR_BIT)==0)
				 printf("\n");
				 printf("%d",bitarray_test_bit(bitarray,i));
			}
	printf("\n");
	free(buffer);
	bitarray_destroy(bitarray);


}
//\n

int calcularCantidadBloques(size_t size){
	if((size%sizeof(int))!=0){return -1;}
	return size/size;
}
/*
	int main(){
	 crear_Archivo_Bitmap();
	 verArrayBits();
	 (reservarBloquesRequeridos(120)<0)? printf("Sorry no se pudo reservar\n"):printf("Reservo como un campeon\n");
	 verArrayBits();
	 (reservarBloquesRequeridos(2)<0)? printf("Sorry no se pudo reservar\n"):printf("Reservo como un campeon\n");
	 verArrayBits();
}*/
