#include <stdio.h>
#include "parser.h"
/* Funcion: parsear_medicion
 * Parametros de Entrada: linea de texto en formato CSV, puntero a Medicion
 * Valor de salida: 1 si el parseo fue exitoso o 0 si la linea es invalida
 * Descripcion: Descompone una linea con el formato estacion,humedad,rocio,
 * presion,hora y llena los campos del struct Medicion, retorna 0 si no se
 * lograron leer los 5 campos esperados */
int parsear_medicion( char *linea, Medicion *m) {
 int leidos = sscanf(linea,"%99[^,],%d,%d,%d,%99[^\n]",m->estacion,&m->humedad,&m->rocio,&m->presion,m->hora);
  if (leidos != 5) {
    return 0;
 }
 return 1;
}
