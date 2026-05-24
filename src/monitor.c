#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include "comun.h"
#include "buffer.h"
#include "parser.h"

typedef struct EstadoEstacion {
    char nombre[MaxEsta];
    Buffer buffer;
    int contador;
    int suma_humedad;
    int suma_rocio;
    int suma_presion;
    char ultima_hora[MaxHora];
    int agente_termino;
} EstadoEstacion;

EstadoEstacion estaciones[NumEstaciones];
pthread_mutex_t mutex_archivo;
FILE *archivo_consolidado;
int fd_pipe;
int tamBuffer;

int buscar_estacion(char *nombre) {
 for (int i = 0; i < NumEstaciones; i++) {
   if (strcmp(estaciones[i].nombre,nombre) ==0) {
    return i;
  }
 }
return -1;
}

int hora_a_segundos(char *hora) {
    int h,mi,s;
    if (sscanf(hora,"%d:%d:%d",&h,&mi,&s) != 3) {
        return -1;
    }
    return h *3600L +mi*60L+s;
}

void *hilo_recolector(void *arg) {
  (void)arg;
char acumulador[8192];
    int posicion = 0;
    char temp[4096];

while(true) {
 ssize_t bytes_leidos= read(fd_pipe,temp,sizeof(temp));
 if (bytes_leidos == 0) {
  break;
 }
 if (bytes_leidos == -1) {
  if (errno == EINTR) {
   continue;
  }
  perror("Error en read del pipe");
  break;
 }
 if (posicion + (int)bytes_leidos >= (int)sizeof(acumulador) - 1) {
  fprintf(stderr,"Acumulador desbordado\n");
  posicion = 0;
  continue;
 }
 memcpy(acumulador + posicion,temp,bytes_leidos);
 posicion += (int)bytes_leidos;
 acumulador[posicion] = '\0';

 char *nl;

 while ((nl = strchr(acumulador,'\n')) != NULL) {
  *nl = '\0';
  char linea[MaxLin];
  strncpy(linea,acumulador,MaxLin - 1);
  linea[MaxLin - 1] = '\0';
  int consumidos = (int)(nl - acumulador) + 1;
  int resto = posicion-consumidos;
  memmove(acumulador,nl + 1,resto);
  posicion = resto;
  acumulador[posicion] = '\0';
  Medicion m;
  if(!parsear_medicion(linea,&m)) {
   fprintf(stderr,"Linea invalida en recolector: %s\n",linea);
   continue;
  }
  int est = buscar_estacion(m.estacion);
  if (est == -1) {
   fprintf(stderr,"Estacion desconocida %s\n",m.estacion);
   continue;
  }

  if (sem_wait(&estaciones[est].buffer.espacios_libres) == -1) {
   perror("sem_wait espacios_libres");
   continue;
  }

  pthread_mutex_lock(&estaciones[est].buffer.mutex_buffer);
  estaciones[est].buffer.datos[estaciones[est].buffer.in] = m;
  estaciones[est].buffer.in = (estaciones[est].buffer.in + 1) %estaciones[est].buffer.tam;
  pthread_mutex_unlock(&estaciones[est].buffer.mutex_buffer);
  if (sem_post(&estaciones[est].buffer.items_disponibles) == -1) {
   perror("sem_post items_disponibles");
  }
 }
}

for (int i = 0; i < NumEstaciones; i++) {
 estaciones[i].agente_termino = 1;
 if (sem_post(&estaciones[i].buffer.items_disponibles) == -1) {
  perror("sem_post terminacion");
 }
}

 return NULL;
}

void *hilo_consumidor(void *arg) {

int idx = *(int *)arg;
EstadoEstacion *est = &estaciones[idx];
while (true) {
 if (sem_wait(&est->buffer.items_disponibles) == -1) {
  if (errno == EINTR) {
   continue;
  }

  perror("sem_wait items_disponibles");

  break;
 }

 if (est->agente_termino) {
  pthread_mutex_lock(&est->buffer.mutex_buffer);
  int vacio = (est->buffer.in == est->buffer.out);
  pthread_mutex_unlock(&est->buffer.mutex_buffer);
  if (vacio) {
   break;
  }
 }

 Medicion med;
 pthread_mutex_lock(&est->buffer.mutex_buffer);
 med = est->buffer.datos[est->buffer.out];
 est->buffer.out = (est->buffer.out + 1) %est->buffer.tam;
 pthread_mutex_unlock(&est->buffer.mutex_buffer);
 if (sem_post(&est->buffer.espacios_libres) == -1) {
  perror("sem_post espacios_libres");
 }

 pthread_mutex_lock(&mutex_archivo);
 fprintf(archivo_consolidado,"%s,%d,%d,%d,%s\n",med.estacion,med.humedad,med.rocio,med.presion,med.hora);
 fflush(archivo_consolidado);
 pthread_mutex_unlock(&mutex_archivo);


 est->suma_humedad += med.humedad;
 est->suma_rocio += med.rocio;
 est->suma_presion += med.presion;
 est->contador += 1;

 if (est->ultima_hora[0] != '\0') {

  int t_actual = hora_a_segundos(med.hora);
  int t_prev = hora_a_segundos(est->ultima_hora);

  if (t_actual >= 0 && t_prev >= 0) {

   int diff = t_actual - t_prev;

   if (diff < 0) {
    diff += 86400;
   }

   if (diff >= 7200) {
    printf("Datos faltantes en estacion %s\n",est->nombre);
   }
  }
 }

 strncpy(est->ultima_hora,med.hora,MaxHora - 1);
 est->ultima_hora[MaxHora - 1] = '\0';
}

 return NULL;
}

char *generar_reporte(void) {
int promH_global = 0;
int promR_global = 0;
int promP_global = 0;
int estaciones_validas = 0;

 for (int i = 0; i < NumEstaciones; i++) {
  if (estaciones[i].contador > 0) {
   promH_global += estaciones[i].suma_humedad / estaciones[i].contador;
   promR_global += estaciones[i].suma_rocio / estaciones[i].contador;
   promP_global += estaciones[i].suma_presion / estaciones[i].contador;
   estaciones_validas++;
  }
 }

 if (estaciones_validas == 0) {
  return "Sin datos";
 }

int promH = promH_global / estaciones_validas;
int promR = promR_global / estaciones_validas;
int promP = promP_global / estaciones_validas;

printf("Humedad=%d rocio=%d presion=%d\n",promH,promR,promP);

if (promH > 90 && promR > 9 && promP < 750) {
 return "Lluvioso";
}
if (promH >= 80 && promH <= 95 && promR > 8 && promP >= 750 && promP <= 752) {
 return "Nublado";
}
if (promH < 80 && promR >= 5 && promR <= 8 && promP > 754) {
 return "Fresco";
}
 return "Indeterminado";
}

int main(int argc,char *argv[]) {
char *nombrePipe = NULL;
tamBuffer = 0;

if (signal(SIGPIPE,SIG_IGN) == SIG_ERR) {
 perror("Error al ignorar SIGPIPE");
 return 1;
}

for (int i = 1; i < argc; i++) {
 if (strcmp(argv[i],"-b") == 0) {
  if (i+1 < argc) {
   tamBuffer = atoi(argv[i + 1]);
   i++;
  }

  else {
   fprintf(stderr,"Uso: %s -b tamBuffer -p nombrePipe\n",argv[0]);
   return 1;
  }
 }

 else if (strcmp(argv[i],"-p") == 0) {

  if (i+1 < argc) {
   nombrePipe = argv[i + 1];
   i++;
  }

  else {
   fprintf(stderr,"Uso: %s -b tamBuffer -p nombrePipe\n",argv[0]);
   return 1;
  }
 }

 else {
  fprintf(stderr,"Flag no reconocido: %s\n",argv[i]);
  fprintf(stderr,"Uso: %s -b tamBuffer -p nombrePipe\n",argv[0]);
  return 1;
 }
}

if (nombrePipe == NULL || tamBuffer <= 0) {
 fprintf(stderr,"Error: faltan argumentos o tamBuffer invalido.\n");
 fprintf(stderr,"Uso: %s -b tamBuffer -p nombrePipe\n",argv[0]);
 return 1;
}

if (mkfifo(nombrePipe,0666) == -1 && errno != EEXIST) {
 perror("Error al crear FIFO");
 return 1;
}

char *nombres[NumEstaciones] = {"EK","ET","EU"};
for (int i = 0; i < NumEstaciones; i++) {
 strncpy(estaciones[i].nombre,nombres[i],MaxEsta - 1);
 estaciones[i].nombre[MaxEsta - 1] = '\0';

 if (!inicializarRecursos(&estaciones[i].buffer,tamBuffer)) {
  fprintf(stderr,"Error inicializando buffer de estacion %d\n",i);
  return 1;
 }

 estaciones[i].contador = 0;
 estaciones[i].suma_humedad = 0;
 estaciones[i].suma_rocio = 0;
 estaciones[i].suma_presion = 0;
 estaciones[i].ultima_hora[0] = '\0';
 estaciones[i].agente_termino = 0;
}

if (pthread_mutex_init(&mutex_archivo,NULL) != 0) {
 perror("Error inicializando mutex_archivo");
 return 1;
}

archivo_consolidado = fopen("consolidado.csv","w");

if (archivo_consolidado == NULL) {
 perror("Error abriendo consolidado.csv");
 return 1;
}

fd_pipe = open(nombrePipe,O_RDONLY);

if (fd_pipe == -1) {
 perror("Error abriendo FIFO en lectura");
 fclose(archivo_consolidado);
 return 1;
}

printf("Control de Categorizacion Meteorologica\n");

pthread_t tid_recolector;
pthread_t tid_consumidores[NumEstaciones];

int indices[NumEstaciones];

if (pthread_create(&tid_recolector,NULL,hilo_recolector,NULL) != 0) {
 perror("Error creando recolector");
 return 1;
}

for (int i = 0; i < NumEstaciones; i++) {
 indices[i] = i;
 if (pthread_create(&tid_consumidores[i],NULL,hilo_consumidor,&indices[i]) != 0) {
  perror("Error creando consumidor");
  return 1;
 }
}

if (pthread_join(tid_recolector,NULL) != 0) {
 perror("pthread_join recolector");
}

for (int i = 0; i < NumEstaciones; i++) {
 if (pthread_join(tid_consumidores[i],NULL) != 0) {
  perror("pthread_join consumidor");
 }
}

const char *categoria = generar_reporte();
printf("Parte Meteorologico Bogota \"%s\"\n",categoria);
printf("Fin del Monitor\n");

if (fclose(archivo_consolidado) != 0) {
 perror("fclose consolidado");
}

if (close(fd_pipe) == -1) {
 perror("close fd_pipe");
}

if (unlink(nombrePipe) == -1) {
 perror("unlink pipe");
}

for (int i = 0; i < NumEstaciones; i++) {
 destructorRecursos(&estaciones[i].buffer);
}

pthread_mutex_destroy(&mutex_archivo);

return 0;
}
