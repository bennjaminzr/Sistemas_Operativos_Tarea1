# Planificador dieciochero

Tarea 1 de Sistemas operativos: simulador y planificador de activiades modeladas como un grafo aciclico dirigido (DAG),
usnado procesos pipes y señales.

Integrantes : Matias Marchant y Bnejamin Aguilera

## Compilacion
 Requiere g++

 ```bash
 make
 ```

 Esto ejecuta `g++ -Wall -Wextra -std=c++17 ... -lpthread`. Para limpiar el ejecutable:

 ```bash
 make clean
 ```

## Ejecucion 
 
 ```bash
 ./planificador plan.txt K
 ```
 
 - `plan.txt`: archivo con las actividades.
 - `K`: límite de concurrencia (entero mayor o igual a 1).

## Formato de ´plan.txt´

 ```
 ID : nombre : tiempo_ms : dep1, dep2, ...
 ```

### Parseo de `plan.txt`

 - `struct Actividad` (`plan.hpp`): guarda el `id`, el `nombre`, el `tiempo_ms`
  y la lista de dependencias (`deps`) de una actividad.
 - `recortar`: elimina espacios, tabs y saltos de línea al inicio y al final de un texto.
 - `dividir`: parte un texto por un carácter separador, conservando los campos vacíos.
 - Lectura en `main`: lee el archivo línea por línea, ignora las líneas vacías,
  divide cada línea en 4 campos y construye una `Actividad`.