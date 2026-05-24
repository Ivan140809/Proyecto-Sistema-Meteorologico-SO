#include <stdlib.h>
#include "buffer.h"

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

void destructorRecursos(Buffer *b) {
 sem_destroy(&b->espacios_libres);
 sem_destroy(&b->items_disponibles);
 pthread_mutex_destroy(&b->mutex_buffer);
 free(b->datos);
}
