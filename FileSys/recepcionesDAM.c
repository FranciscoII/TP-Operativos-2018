#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "gs.h"
#include <semaphore.h>
#include"comunicacionDAM.h"
#include"instrucciones.h"
#include"inicializacionMDJ.h"
#include<commons/config.h>

extern int DAM;
extern pthread_mutex_t lock;

void recepcionesDAM(){

	int a=0;

	while(1){
			recv(DAM,&a,4,MSG_WAITALL);
		pthread_mutex_lock(&lock);
			recibirInstruccionYAccionar(logger,DAM);
		pthread_mutex_unlock(&lock);
		}

}




