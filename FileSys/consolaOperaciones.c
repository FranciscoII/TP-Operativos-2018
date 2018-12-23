/*#include"consolaOperaciones.h"
char* DIRECTORIOACTUAL;

t_config *configu;


char tipoDeArchivoRutaRelativa(char* rutaRelativa){
	char* rutaAbsoluta=rutaRelativaPasajeaAbsoluta(rutaRelativa);
	int a=tipoDeArchivo(rutaAbsoluta);
	free(rutaAbsoluta);
	return a;
}
void configuroDirectorioActual(){
	configu =config_create("/home/utnso/workspace"
				"/tp-2018-2c-Operativos-Crusaders/CONFIG/MDJ.txt");
	char* dire=config_get_string_value(configu,"PUNTO_MONTAJE");
	int l=strlen(dire)+1;
	DIRECTORIOACTUAL=malloc(l);
	memcpy(DIRECTORIOACTUAL,dire,l);

}
//ingresas parte de la ruta y veo si al volverla absoluta existe
int comprueboRutaRelativa(char* rutaRelativa){
	char*rutaAbsoluta= rutaRelativaPasajeaAbsoluta(rutaRelativa);
	int a= tipoDeArchivo(rutaAbsoluta);
	printf("%s\n",rutaAbsoluta);
	free(rutaAbsoluta);
	return a;
}
//SI LA RUTA EMPIEZA CON BARRITA SIGNIFICA QUE ES ABSOLUTA
int tipoDeRuta(char *ruta){
	return ruta[0]=='/';
}
char* rutaRelativaPasajeaAbsoluta(char* rutaRelativa){
	int l=strlen(DIRECTORIOACTUAL)+1;
	char *ruta=malloc(l);
	memcpy(ruta,DIRECTORIOACTUAL,l);
	string_append(&ruta,"/");
	string_append(&ruta,rutaRelativa);
	return ruta;
}
//CUALQUIER NEGATIVO HACE QUE NO SEA UN ARCHIVO VALIDO

int tipoDeArchivo(char *rutaAbsoluta){
	struct stat date;
	int respuesta=-2;
	if(stat(rutaAbsoluta,&date)==-1)
		respuesta= -1;
	if(S_ISDIR(date.st_mode))
		respuesta=1;
	if(S_ISREG(date.st_mode))
		respuesta= 2;
	return respuesta;
}
//EU DA POR ENTENDIDO QUE LE PASAS UNA RUTA VALIDA!
void actualizoDirectorioActual(char* rutaAbsoluta){
	free(DIRECTORIOACTUAL);
	int labsoluta=strlen(rutaAbsoluta)+1;
	DIRECTORIOACTUAL=malloc(labsoluta);
	memcpy(DIRECTORIOACTUAL,rutaAbsoluta,labsoluta);
}
int empiezaConPunto(char* ruta){
	return (ruta[0]=='.');

}
void cd(char* ruta){
	char* absoluta;
	switch(tipoDeRuta(ruta)){
	case 1:
		if(tipoDeArchivo(ruta)==1){
			printf("Ruta: %s\n",ruta);
			actualizoDirectorioActual(ruta);

		}
			break;
	case 0:
		absoluta=rutaRelativaPasajeaAbsoluta(ruta);
			if(tipoDeArchivo(absoluta)==1){
				printf("Ruta: %s\n",absoluta);
			actualizoDirectorioActual(absoluta);
			}
			break;
	default:
		printf("No es directorio o ruta valida\n");

	}

}
int esRutaValida(char* ruta){
	if(tipoDeRuta(ruta)==0){
		char *absoluta=rutaRelativaPasajeaAbsoluta(ruta);
		int a= tipoDeArchivo(absoluta)>0;
		free(absoluta);
		return a;
	}
	else
		return tipoDeArchivo(ruta)>0;
}
int esDirectorio(char *ruta){
	if(tipoDeRuta(ruta)==0){
		char *absoluta=rutaRelativaPasajeaAbsoluta(ruta);
		int a=(tipoDeArchivo(absoluta)==1);
		free(absoluta);
		return a;
	}
	else
		return tipoDeArchivo(ruta)==1;

}



void listarDirectorio(char* posible){
	char* dire=string_new();
	string_append(&dire,DIRECTORIOACTUAL);
	if(posible!=NULL){
		if((esRutaValida(posible)>0)&&(esDirectorio(posible))){
			cd(posible);
		}
		else{
			cd(DIRECTORIOACTUAL);
			return;
		}
	}
	DIR* dir;
	struct dirent*ent;
	dir=opendir(DIRECTORIOACTUAL);
	if(dir==NULL){
		printf("No se pudo acceder al directorio");
	}
	while((ent=readdir(dir))!=NULL){

	if((strcmp(ent->d_name,".")!=0) &&(strcmp(ent->d_name,"..")!=0)){
		if ( tipoDeArchivoRutaRelativa(ent->d_name)==1)//\x1B35
			printf("\x1b[35m%s\n\x1b[0m",ent->d_name);
		if (tipoDeArchivoRutaRelativa(ent->d_name)==2)//\x1B32
			printf("\x1b[32m%s\n\x1b[0m",ent->d_name);
				//printf("\x1b[32m%s\n\x1b[0m",ent->d_name);
				}
	}
	free(DIRECTORIOACTUAL);
	DIRECTORIOACTUAL=string_new();
	string_append(&DIRECTORIOACTUAL,dire);
	free(dire);
	closedir(dir);
	free(ent);
}

void obtenerTotalidadArchivo(char*path){
	if(verificarValidacion(path)<0){
		printf("NO EXISTE EL ARCHIVO");
		return;}
	int size=sizeArchivo(path);
	devMDJ *dev= obtenerDatos(path,0,size);
	char buffer[size+1];
	memcpy(buffer,dev->buffer,size);
	free(dev->buffer);
	free(dev);
	buffer[size]='\0';
	printf("CONTENIDO DEL ARCHIVO %s:\n %s \n",path,buffer);
}

void cat(char *path){
	char* archivo=string_new();
	if(esRutaValida(path)<0){
		printf("RUTA INVALIDA");
		return;
	}
	if(string_contains(path,"/")){
		char**array=string_split(path,"/");
		int i;
		for(i=0;i<array[i]!=NULL;i++);
		string_append(&archivo,array[i-1]);
		for(int j=(i-1);i>=0;i--)
			free(array[i]);
		free(array);
	}
	else
		string_append(&archivo,path);

		obtenerTotalidadArchivo(archivo);
		free(archivo);
}
*/
/*
void md5(char*path){
	char*patheando;
	if(esCaminoValido(path)<0){
	patheando=crearCamino1(path);
	if(esCaminoDeArchivo(path)<0){
		printf("No existe el bloque\n");
		return;
	}
	}
	else if(esCaminoDeArchivo(path))
	patheando=path;
	char*archivo=obtenerTotalidadArchivo(patheando);
	void * digest = malloc(MD5_DIGEST_LENGTH);
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, archivo, strlen(archivo) + 1);
	MD5_Final(digest, &context);
	char* md5[MD5_DIGEST_LENGTH+1];
	md5[MD5_DIGEST_LENGTH]='\0';
	memcpy(md5,digest,MD5_DIGEST_LENGTH);
	printf("\n%s\n",md5);
	printf("\n");
}
*/



/*
char* analizadorDosPuntos(char*rutaAbsoluta){
	char**cortandoCamino=string_split(rutaAbsoluta,"/");
	int i;
	//Cuenta la cantidad de elementos
	for(i=0;cortandoCamino[i]!=NULL;i++);
	int cantidadDeElementos=i;
	for(i=0;cortandoCamino[i]!=NULL;i++){
		printf("%s\n",cortandoCamino[i]);
	}
	printf("Cantidad de Elementos: %d\n",cantidadDeElementos);
	char*auxiliar=string_new();
	int h=0;
	while(h<cantidadDeElementos){
		int j=h;
		if((string_contains(cortandoCamino[j],"..")&&(string_contains(cortandoCamino[j],".."))){

		}

		for(j=h;(j<cantidadDeElementos-1)&&(string_contains(cortandoCamino[j+1],"..")==0);j++){
			string_append(&auxiliar,"/");
			string_append(&auxiliar,cortandoCamino[j]);
		}
		h=j;
	printf("%s \n",auxiliar);

	if(h==cantidadDeElementos-1){
		string_append(&auxiliar,"/");
		string_append(&auxiliar,cortandoCamino[h]);
	}
	printf("%s \n",auxiliar);

	if((h<cantidadDeElementos-1)&&(string_contains(cortandoCamino[h+1],"..")))
		h++;
	h+=1;
	printf("%d\n",h);
		}
	for(int j=cantidadDeElementos-1;j>=0;j--)
		free(cortandoCamino[j]);
	free(cortandoCamino);
	return auxiliar;
}
*/


