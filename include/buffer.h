#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <semaphore.h>
#include "comun.h"

typedef struct Buffer {
    Medicion *datos;
    int tam;
    int in;
    int out;
    sem_t espacios_libres;
    sem_t items_disponibles;
    pthread_mutex_t mutex_buffer;
} Buffer;

int buffer_init(Buffer *b, int tam);
void buffer_destroy(Buffer *b);

#endif

