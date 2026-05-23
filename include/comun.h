#ifndef COMUN_H
#define COMUN_H

#define NumEstaciones 3
#define MaxLin 256
#define MaxEsta 100
#define MaxHora 100
#define HumMin 77
#define HumMax 100
#define PreMin 740
#define PreMax 760
#define RocMin 3
#define RocMax 12

typedef struct Medicion{
    char estacion[MaxEsta];
    int humedad;
    int rocio;
    int presion;
    char hora[MaxHora];
} Medicion;

#endif

