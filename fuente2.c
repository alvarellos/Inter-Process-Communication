// fuente2.c

#include <errno.h>
#include <fcntl.h>  
#include <limits.h> 
#include <libgen.h> 
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
char *ejecutable = "Ej2";
int proceso;

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

	// Proceso 2
	proceso = 2;

	// 1. Lee el fichero FIFO "fichero1"
	describeProceso("Abriendo fichero FIFO fichero1"); 
	int fichero = open("fichero1", O_RDWR | O_NONBLOCK); 
	if (fichero < 0)  { 
		perror("open");
		exit(1);
	}  
		
	describeProceso("Leyendo mensaje de FIFO fichero1"); 
	int longitud; 
	char mensaje[MAX]; 
	
	if ((longitud = read(fichero, mensaje, MAX)) < 0)  { 
		perror("read");
		exit(1);
	} 
	describeProceso("Proceso de lectura finaliza con éxito:");
	printf(" ", longitud); 
	int i; 
	
	for (i = 0; i < longitud; i++) printf("%c", mensaje[i]);	
		printf("\n");

	// 2. Crea una región de Memoria compartida
	describeProceso("Creando region de Memoria Compartida");
	int key = ftok("fichero1", 'i'); // Clave 2
	int id_semaforo = semget(key, 1, 0777 | IPC_CREAT); // Reserva 1 semáforo
	int id_memoria;
	
	if (id_memoria = shmget(key, MAX * sizeof (int), IPC_CREAT | 0777) < 0)  { 
		perror("shmget");
		exit(1);
	} 
	
	// 3. Almacena variable "vcl"
	int *vc1 = (int *) shmat(id_memoria, 0, 0);
	describeProceso("Cerrando Semaforo para P3");

	// 4. Almacena un semáforo para proteger el acceso a dicha región de memoria
	union { 
		int val; 
		struct semid_ds *buf; 
		ushort *array; 
	} arg;

	arg.val = 0;
	semctl(id_semaforo, 0, SETVAL, arg); // inicializa Semaforo a 0.
	struct sembuf op_V = {0, +1, 0}; // sumar uno al semaforo : V())
	struct sembuf op_P = {0, -1, 0}; // restar uno al semaforo : P())
	
	semop(id_semaforo, &op_V, 1); // +1 : lo pone en “rojo”

	describeProceso("Bifurcacion en P2 y P3");

	// 5. Bifurcación de P2 y P3 (hijo)

	pid_t pid_hijo;

	if ((pid_hijo = fork()) == -1)  { 
		perror("fork");
		exit(1);
	} 
	
	if (pid_hijo != 0) {

		// Proceso P2
		proceso = 2;

		// 5.1. El proceso P2 Espera un segundo (continúa P3)
		describeProceso("Espera (1 segundo)");
		sleep(1);
		describeProceso("Fin de espera (1 segundo)");

		// 5.2 Escribe mensaje en variable vc1
		describeProceso("Escribe Mensaje en Memoria Compartida con P3");

		vc1[0] = longitud; // guarda longitud
		int j;
		for (j = 0; j < longitud; j++) vc1[j + 1] = (int) mensaje[j];

		// 5.3. Operación para abrir el semáforo
		describeProceso("Abre semaforo");
		semop(id_semaforo, &op_P, 1); // -1 : lo pone en “verde”

		// 5.4 Espera destrucción de proceso P2
		describeProceso("Espera a ser finalizado (pause)");
		pause();


	} else {
		// 5.5. Proceso3. Se ejecuta Ej3
		proceso = 3;
		execv("./Ej3", NULL);
	}

	// Es matado por P1 antes de llegar al fin
	return 0;

} // fin fuente2