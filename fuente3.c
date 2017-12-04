// fuente3.c 

#include <errno.h> 
#include <fcntl.h> 
#include <libgen.h> 
#include <limits.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <signal.h> 
#include <sys/msg.h> 
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h> 
#include <time.h> 
#include <unistd.h> 

const int MAX = 200;
int proceso; 
int *siguiente;
char *ejecutable = "Ej3";

void describeProceso(char *texto) {

	switch (proceso) {
	case 1 :
		printf("\e[1m\e[33m Proceso P%i (PID=%i, %s) %s\n", proceso, getpid(), ejecutable, texto);
		printf("\e[1m\e[37m");
	break;
	case 2 :
		printf("\e[1m\e[34m Proceso P%i (PID=%i, %s) %s\n", proceso, getpid(), ejecutable, texto);
		printf("\e[1m\e[37m");
	break;
	case 3: 
		printf("\e[1m\e[35m Proceso P%i (PID=%i, %s) %s\n", proceso, getpid(), ejecutable, texto);
		printf("\e[1m\e[37m");
	}
};


int main(int argc, char *argv[]) {
	
	// Proceso 3	
	proceso = 3;

	// 3.1. Carga la región de memoria compartida (variable vc1 y  semáforo)
	describeProceso("Carga región de memoria compartida con P2");
	int LlaveMemoria = ftok("fichero1", 'i'); // Clave 2
	int id_semaforo = semget(LlaveMemoria, 1, 0666);
	int id_memoria;

	// 3.2. Se pide la zona reservada
	if (id_memoria = shmget(LlaveMemoria, MAX * sizeof (int), SHM_R | SHM_W) < 0)  { 
		perror("shmget");
		exit(1);
	}

	// 3.3. Asocia con “vc1”	
	int *vc1 = (int *) shmat(id_memoria, 0, 0);

	describeProceso("Esperando semáforo de P2 (indica memoria libre)");
	// 3.4. Se espera que esté el semáforo en verde
	while (semctl(id_semaforo, 0, GETVAL, 0) > 0);

	describeProceso("Semáforo liberado. Leyendo mensaje de P2");
	
	// 3.5. Mostrar mensaje por pantalla
	char mensaje[MAX];
	int longitud = vc1[0];
	describeProceso("Proceso de lectura finaliza con éxito. Mensaje:");
	printf(" ", longitud);
	int j;
	for (j = 0; j < longitud; j++) mensaje[j] = (char) vc1[j + 1];
	
	// Arregla problema de espacios en blanco finales
	mensaje[longitud] = '\0';

	// Imprime el mensaje
	printf("%s \n", mensaje);

	// 3.6. Se libera la memoria compartida
	// Tras la última ejecución habrá que hacerlo manualmente

	shmdt(siguiente);
	describeProceso("Memoria compartida liberada");

	// 3.7. Se libera semáforo
	semctl(id_semaforo, 0, IPC_RMID);
	describeProceso("Semáforo liberado");

	// 3.8. Se envía mensaje a la cola para P1

	describeProceso("Enviando mensaje a cola");

	struct { 
		long tipo; 
		int numPID;
	} mensajeCola; 

	int cola;
	key_t key;
	int longitudMensaje=sizeof(mensajeCola)-sizeof(mensajeCola.tipo);

    	if ((key = ftok("Ej1", 'i')) == -1) {
        	perror("ftok");
        	exit(1);
    	}

    	if ((cola = msgget(key, 0666 | IPC_CREAT)) == -1) {
        	perror("msgget");
        	exit(1);
    	}

	mensajeCola.tipo = 1;
	mensajeCola.numPID = getpid();

	// Envia el PID
	if (msgsnd(cola, &mensajeCola, longitudMensaje, IPC_NOWAIT) == -1) 
		perror("msgsnd");
	describeProceso("Mensaje enviado");


	// 3.9. P3 espera su destrucción
	describeProceso("Espera a ser finalizado (pause)");
	pause();

	// Es matado por P1 antes de llegar al fin
	return 0;

} // fin fuente3