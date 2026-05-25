#ifndef PARSER_H
#define PARSER_H

#include "comun.h"
/* Funcion: parsear_medicion
 * Parametros de Entrada: linea de texto en formato CSV, puntero a Medicion
 * Valor de salida: 1 si el parseo fue exitoso o 0 si la linea es invalida
 * Descripción: Descompone una linea con el formato estacion,humedad,rocio,
 * presion,hora y llena los campos del struct Medicion, retorna 0 si no se
 * lograron leer los 5 campos esperados */
int parsear_medicion(char *linea, Medicion *m);

#endif

