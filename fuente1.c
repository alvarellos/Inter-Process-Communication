// fuente1.c 

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
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

// Para la cola


const int MAX = 200;
const int leerTuberia = 0;
const int escribirTuberia = 1;
char *ejecutable = "Ej1";
int proceso = 0;


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

	
	// Obtener filename desde el path
	char *path = argv[0]; 
	char *ssc; 
	int l = 0; 
	ssc = strstr(path, "/"); 
	do { 
		l = strlen(ssc) + 1; 
		path = &path[strlen(path) - l + 2]; 
		ssc = strstr(path, "/");
	} while (ssc);

	ejecutable = path;

	int tuberia[2];
	
	// Se inicia tubería para P1 y P2
	pipe(tuberia);

	// Bifurcación de P1 y P2 (hijo)

	pid_t pid_hijo; 
	if ((pid_hijo = fork()) == -1) { 
		perror("fork");
		exit(1);
	} 
	
	if (pid_hijo != 0) {

		// Proceso P1
		proceso = 1;

		// Cálculo CPU
		struct tms st_cpu;
		clock_t st_time = times(&st_cpu);

		// Tubería
		close(tuberia[leerTuberia]);

		// 1. Lectura de mensaje por teclado
		describeProceso("Pide mensaje para enviar a P2"); 
		char mensaje[MAX];
		fgets(mensaje, MAX, stdin);

		// 2. Envío del mensaje por la tubería a proceso P2
		describeProceso("Enviando Mensaje por tuberia sin nombre a P2"); 
		strcat(mensaje, "FIN"); // para saber cuando acaba.
		if (write(tuberia[escribirTuberia], mensaje, (strlen(mensaje))) < 0){
			perror("write");
			exit(1);
		}

		// 3. Crear Cola de mensajes asociada a Ej1 que utiliza P3
		describeProceso("Creando cola de mensajes"); 

		struct {
			long tipo;
			int numPID;
		} mensajeCola;

		int cola;
		key_t key;
		int longitudMensaje=sizeof(mensajeCola)-sizeof(mensajeCola.tipo);

		// Llave para P1 y P3
		if ((key = ftok("Ej1", 'i')) == -1) { 
       			perror("ftok");
       			exit(1);
		}

		// Realizar la conexión a la cola
		if ((cola = msgget(key, 0666 | IPC_CREAT)) == -1) {
       			perror("msgget");
       			exit(1);
		}

		// 4. Espera de mensaje de proceso P3 (sigue P2 y luego P3)
		describeProceso("P1 se queda a la espera de recibir un mensaje");
		sleep(3);

		mensajeCola.tipo = 1;

		// 5. Recibe mensaje de proceso P3 y lo escribe
		if (msgrcv(cola, &mensajeCola, longitudMensaje, mensajeCola.tipo, IPC_NOWAIT) == -1) {
            		perror("msgrcv");
            		exit(1);
		}
		describeProceso( "Mensaje recibido con éxito de Cola ");
		// printf(" Recibido mensaje de la cola: \"%d\"\n", mensajeCola.numPID);

		int pid_P3 = mensajeCola.numPID;
		printf(" PID de P3 = %i\n", pid_P3);

		// 6. Envio de señal de SIGKILL a P2 y a P3


		if (kill(pid_hijo, SIGKILL) < 0) {
                 	perror("kill");
			exit(1);	
		}
		describeProceso("Eliminado proceso P2");

		if (kill(pid_P3, SIGKILL) < 0) { 
			perror("kill");
			exit(1);			
		}
		describeProceso("Eliminado proceso P3");

		// 7. Borrar el fichero1

		if (unlink("fichero1") < 0) { 
			perror("unlink");
			exit(1);
		}
		describeProceso("Borrado -fichero1- ");

		// 8. Muestra estadísticas del uso del CPU
		struct tms en_cpu; // calcular el incremento: actual-anterior
		clock_t en_time = times(&en_cpu);

		printf(" Tiempo Real: %i ms \n", (int) (en_time - st_time));
		printf(" Tiempo de Usuario: %i ms\n", (int) (en_cpu.tms_utime - st_cpu.tms_utime));
		printf(" Tiempo del Sistema: %i ms\n", (int) (en_cpu.tms_stime - st_cpu.tms_stime));

	} else{

		// Proceso P2

		proceso = 2;
		
		// Tubería
		close(tuberia[escribirTuberia]);

		// 1. Lee la tuberia sin nombre
		describeProceso("Espera mensaje de P1 por tuberia sin nombre");

		// Lee mensaje
		char mensaje[MAX]; 
		int longitud = read(tuberia[leerTuberia], mensaje, MAX) - 4;
		
		// Imprime por pantalla el mensaje		
		describeProceso("Recibido el mensaje de P1: ");
		printf(" ", longitud);

		int i;
		for (i = 0; i <= longitud; i++) printf("%c", mensaje[i]);
		

		// 2. Crear fichero FIFO y escribir el mensaje

		if (mknod("fichero1", S_IFIFO | 0666, 0) < 0) { 
			perror("mknod");
			exit(1);
		}
		describeProceso("Se crea fichero FIFO: fichero1");

		// Escribe el mensaje
		int fichero = open("fichero1", O_RDWR | O_NONBLOCK);
		if (fichero < 0) { 
			perror("open");
			exit(1);
		}
		describeProceso("Escribiendo en FIFO fichero1");
		if ((longitud = write(fichero, mensaje, longitud)) < 0) { 
			perror("write");
			exit(1);
		}
		printf(" Proceso de escritura finaliza con éxito", longitud);

		// 3. Ejecuta Ej2
		execv("./Ej2", NULL);

	}

	return 0; // fin fuente1
}
