
#ifndef CONSOLAMDJ_H_
#define CONSOLAMDJ_H_

char* obtenerArchivoDeLaRuta(char *path);
char* devolverRutaAbsoluta(char *path);
int comprueboCaminoArchivo(char* path);
int dentroDeLosLimites(char* ruta);
int tipoDeArchivo(char *name);
void md5(char* archivo);
int irHastaBarra(char* path,int i);
int cantidadDeBarrass(char* path);
char* irHastaContenido(char* path,int i);
char *obtenerArchivoString(char*path);
void printMDJ(char*linea);


#endif /* CONSOLAMDJ_H_ */


