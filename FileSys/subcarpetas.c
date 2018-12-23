#include "subcarpetas.h"
extern t_config *config;

void crearSubcarpetas(char*path){
		int cantbarritas=cantidadDeBarras(path);
		for(int i=1;i<=cantbarritas;i++){
			char * cam = irHastaContenidoN(path,i);
			char* carpeta=pathCompleto(cam);
			printf("%s \n",carpeta);
			mkdir(carpeta,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
			free(carpeta);
			free(cam);
		}

	}

int cantidadDeBarras(char* path){
	int i,contador;
	contador=0;
	for(i=0;i<strlen(path);i++)
		if(path[i]=='/')
			contador++;
	return contador;
}
int irHastaBarraN(char* path,int i){
	int c=0;
	while((i>0)&&(c<strlen(path))){
			if(path[c]=='/')
				i--;
			c++;
		}
		return c-1;

}
char* irHastaContenidoN(char* path,int i){
	char* camino=string_substring(path,0,irHastaBarraN(path,i));
	return camino;
}
char *obtenerArchivo(char*path){
	char* camino=string_substring(path,irHastaBarraN(path,cantidadDeBarras(path))+1,strlen(path));
	return camino;
}
char* pathCompleto(char*path){
	char* completo=string_new();
	char*anterior=config_get_string_value(config,"PUNTO_MONTAJE");
	string_append(&completo,anterior);
	string_append(&completo,"/Archivos");
	string_append(&completo,path);
	return completo;
}


