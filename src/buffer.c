/* Modulo de inicializacion y destruccion de recursos de sistema
* Archivo: buffer.c
* Autor: I. Lastra, J. Mejia, A. Arunachalam, C. Quintero
* Contiene: Implementa el buffer acotado utilizado por el monitor para la
* comunicacion entre el hilo Recolector y los hilos Consumidores.
* Provee las funciones de inicialización, inserción, extracción y
* destrucción, sincronizadas con semáforos y mutex POSIX.
* Fecha: Mayo 2026
*/
#include <stdlib.h>
#include "buffer.h"
/* Funcion: inicializarRecursos
 * Parámetros de Entrada: puntero a Buffer, tamaño del buffer
 * Valor de salida: 1 si se inicializo correctamente o 0 si fallo
 * Descripción: Reserva memoria para el arreglo de mediciones e inicializa
 * los semaforos y el mutex del buffer */
int inicializarRecursos(Buffer *b, int tam) {
   b->datos = malloc(sizeof(Medicion) * tam);
   if (b->datos == NULL) {
   return 0;
   }
   b->tam = tam;
   b->in = 0;
   b->out = 0;
  if (sem_init(&b->espacios_libres, 0, tam) != 0) {
  free(b->datos);
  return 0;
 }
if (sem_init(&b->items_disponibles, 0, 0) != 0) {
  sem_destroy(&b->espacios_libres);
  free(b->datos);
  return 0;
 }
if (pthread_mutex_init(&b->mutex_buffer, NULL) != 0) {
    sem_destroy(&b->espacios_libres);
    sem_destroy(&b->items_disponibles);
    free(b->datos);
   return 0;
  }
 return 1;
}
/* Funcion: destructorRecrusos
 * Parametros de Entrada: puntero a Buffer
 * Valor de salida: ninguno
 * Descripción: Libera los recursos asociados al buffer: destruye los dos
 * semaforos, destruye el mutex y libera la memoria del arreglo de mediciones */
void destructorRecursos(Buffer *b) {
 sem_destroy(&b->espacios_libres);
 sem_destroy(&b->items_disponibles);
 pthread_mutex_destroy(&b->mutex_buffer);
 free(b->datos);
}
