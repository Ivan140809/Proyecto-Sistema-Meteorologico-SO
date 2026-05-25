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
/* Funcion: inicializarRecursos
 * Parámetros de Entrada: puntero a Buffer, tamaño del buffer
 * Valor de salida: 1 si se inicializo correctamente o 0 si fallo
 * Descripción: Reserva memoria para el arreglo de mediciones e inicializa
 * los semaforos y el mutex del buffer */
int  inicializarRecursos(Buffer *b, int tam);


void destructorRecursos(Buffer *b);

#endif

