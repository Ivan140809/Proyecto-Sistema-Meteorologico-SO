# Proyecto Sistema Meteorológico (SO)

[![C++](https://img.shields.io/badge/C++-11-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/) [![GCC](https://img.shields.io/badge/GCC-g++-A42E2B?style=for-the-badge&logo=gnu&logoColor=white)](https://gcc.gnu.org/) [![Java](https://img.shields.io/badge/Java-11+-orange?style=for-the-badge&logo=openjdk&logoColor=white)](https://openjdk.org/) [![Status](https://img.shields.io/badge/Estado-Finalizado-brightgreen?style=for-the-badge)](https://github.com/)

Repositorio de un sistema simple de adquisición y monitorización meteorológica, usando conceptos de hilos, procesos, comunicación de procesos con pipes nominales, semáforos y mutex.

## Descripción
Este proyecto contiene un conjunto de programas en C para leer datos de sensores, procesarlos y generar un archivo consolidado. Incluye un agente principal (`agenteM`) y una utilidad de monitorización (`monitor`).
## Requisitos
- Linux
- GCC (por ejemplo `gcc`)
- Make
## Compilación
Desde la raíz del repositorio ejecuta:

```sh
make
```
Esto generará los ejecutables `agenteM` y `monitor` en la raíz del proyecto.

Si quieres compilar solo uno de los binarios:
```sh
make agenteM
make monitor
```

## Uso
Después de compilar, ejecuta el agente principal:

```sh
./agenteM
```
O ejecuta la utilidad de monitorización:
```sh
./monitor
```

Las entradas de sensores de ejemplo están en los archivos CSV en la carpeta `data`:
- `sensorEK.csv`
- `sensorET.csv`
- `sensorEU.csv`
El programa produce `consolidado.csv` al ejecutar (y `make clean` lo eliminará).

## Estructura del proyecto
- include/: archivos de cabecera (`buffer.h`, `comun.h`, `parser.h`)
- src/: código fuente (`agenteM.c`, `buffer.c`, `monitor.c`, `parser.c`)
- obj/: objetos compilados (generado por el `Makefile`)
- data/: datos de ejemplo de entrada por cada una de las estaciones (modificable)
- consolidado.csv: archivo de salida generado

## Limpieza
Para limpiar artefactos de compilación y el CSV consolidado:
```sh
make clean
```

## Contribuir
Si quieres contribuir, crea una rama nueva, haz cambios y abre un pull request describiendo los cambios.
