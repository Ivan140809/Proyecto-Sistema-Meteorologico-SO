#include <stdio.h>
#include "parser.h"

int parsear_medicion( char *linea, Medicion *m) {
 int leidos = sscanf(linea,"%99[^,],%d,%d,%d,%99[^\n]",m->estacion,&m->humedad,&m->rocio,&m->presion,m->hora);
  if (leidos != 5) {
    return 0;
 }
 return 1;
}
