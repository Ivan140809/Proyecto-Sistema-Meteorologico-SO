/* Modulo de agente de medicion de las diferentes estaciones
* Archivo: agenteM.cpp
* Autor: I. Lastra, J. Mejia, A. Arunachalam, C. Quintero
* Contiene: Implementa el Agente de Medicion. Lee las mediciones de los sensores
* de una estacion meteorologica desde un archivo CSV, valida que los
* parametros esten en los rangos aceptables
* y transmite las mediciones validas al proceso monitor a través de un pipe nominal
* Fecha: Mayo 2026
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "comun.h"
#include "parser.h"
/* Funcion: main
 * Parámetros de Entrada: cantidad de argumentos y arreglo de argumentos
 * Valor de salida: 0 si termino correctamente o 1 si hubo error
 * Descripción: Procesa los flags -f, -t y -p, abre el archivo de sensores
 * y el pipe hacia el monitor, lee cada linea del archivo y envia las
 * mediciones aceptables al monitor esperando el tiempo indicado entre cada
 * envio, termina al encontrar un punto al inicio de una linea */
int main(int argc, char *argv[]) {
char *archivo = NULL;
char *nombrePipe = NULL;
int tiempo = 0;
int fd_pipe;
FILE *f;
char linea[MaxLin];
Medicion m;
m.estacion[0] = '\0';

  if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
   perror("Error al ignorar SIGPIPE");
    return 1;
  }
  for (int i=1; i<argc; i++) {
   if (strcmp(argv[i], "-f") == 0) {
     if (i+1 <argc) {
      archivo = argv[i+1];
      i++;
   } else {
     fprintf(stderr, "Uso: %s -f archivo -t tiempo -p nombrePipe \n", argv[0]);
       return 1;
     }
  }
   else if (strcmp(argv[i], "-t") == 0) {
     if (i+1<argc) {
      tiempo = atoi(argv[i + 1]);
      i++;
   } else{
      fprintf(stderr, "Uso: %s -f archivo -t tiempo -p nombrePipe\n", argv[0]);
       return 1;
     }
  }
   else if (strcmp(argv[i], "-p") == 0) {
     if (i+1 < argc) {
      nombrePipe = argv[i + 1];
      i++;
      } else {
      fprintf(stderr, "Uso del pipe: %s -f archivo -t tiempo -p nombrePipe\n", argv[0]);
      return 1;
      }
  }
   else {
    fprintf(stderr, "el flag no reconocido: %s\n", argv[i]);
    fprintf(stderr, "Uso del pipe: %s -f archivo -t tiempo -p nombrePipe\n", argv[0]);
            return 1;
        }
    }

 if (archivo == NULL|| nombrePipe == NULL||tiempo <= 0) {
   fprintf(stderr, "Error: faltan argumentos obligatorios o el tiempo es invalido.\n");
   fprintf(stderr, "Uso del pipe: %s -f archivo -t tiempo -p nombrePipe\n", argv[0]);
    return 1;
 }

 f = fopen(archivo, "r");
 if (f == NULL) {
   perror("Error al abrir el archivo de sensores");
   return 1;
  }
  fd_pipe = open(nombrePipe, O_WRONLY);

  if (fd_pipe==-1) {
   perror("Error al abrir el pipe");
   fclose(f);
   return 1;
 }

printf("Agente de Medicion en proceso \n");
 while (fgets(linea, MaxLin, f) != NULL) {
   if (linea[0] == '.') {
     break;
  }
 linea[strcspn(linea, "\n")] = '\0';
     if (!parsear_medicion(linea, &m)) {
            printf("Linea invalida, no se transmite: %s\n", linea);
            sleep(tiempo);
            continue;
  }
int aceptable = 1;
  if (m.humedad< HumMin|| m.humedad>HumMax) {
    aceptable =0;
 }
if (m.presion<PreMin || m.presion > PreMax) {
    aceptable = 0;
}
if (m.rocio<RocMin || m.rocio > RocMax) {
    aceptable = 0;
}

if (aceptable) {
char mensaje[MaxLin];
snprintf(mensaje,MaxLin,"%s,%d,%d,%d,%s\n",m.estacion,m.humedad,m.rocio,m.presion,m.hora);
ssize_t escritos = write(fd_pipe, mensaje, strlen(mensaje));

if (escritos== -1) {
 if (errno == EPIPE) {
   fprintf(stderr, "El monitor cerro el pipe \n");
    break;
 } else {
  perror("Error al escribir en el pipe");
   }
  }
 }
sleep(tiempo);
}
sleep(tiempo);
printf("Fin de Lectura de Sensores de la Estacion %s \n", m.estacion);
fclose(f);
close(fd_pipe);
return 0;
}
