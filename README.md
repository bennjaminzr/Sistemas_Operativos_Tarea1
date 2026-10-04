# Planificador Dieciochero

Tarea 1 de Sistemas Operativos. El programa simula un día de celebración
dieciochera como un grafo de actividades con dependencias (un DAG), donde
cada actividad corre en su propio proceso y el padre se encarga de ir
lanzándolas en el orden correcto, respetando un límite de concurrencia.

**Integrantes:** Matías Marchant y Benjamín Aguilera

## Cómo compilar y ejecutar

Se necesita `g++` con soporte para C++17.

```bash
make
```

El Makefile corre `g++ -Wall -Wextra -std=c++17 ... -lpthread`, tal como pide
el enunciado. `make clean` borra el ejecutable.

Para correrlo:

```bash
./planificador plan.txt K
```

`plan.txt` es el archivo con las actividades y `K` es cuántos procesos pueden
estar vivos al mismo tiempo.

También se puede forzar que algunas actividades fallen, para probar que el
programa aísla bien los errores:

```bash
FALLO_PROB=30 ./planificador plan.txt K
```

Ese `30` es el porcentaje de probabilidad de que cada actividad falle. Si no
se define esta variable, nunca falla nada.

## El formato de plan.txt

Cada línea describe una actividad:

```
ID : nombre : tiempo_ms : dep1, dep2, ...
```

El tiempo puede quedar vacío, y ahí se sortea uno entre 100 y 5000 ms. Las
dependencias también pueden ir vacías si la actividad no espera a nada.

## Qué hace el programa, por partes

**Leer el plan.** El programa lee el archivo línea por línea, separa cada
línea por los `:` con la función `dividir`, y limpia los espacios con
`recortar`. Si una línea no tiene los 4 campos esperados, el programa avisa
en qué línea está el problema y corta la ejecución ahí, en vez de intentar
adivinar.

**Armar el grafo.** Cada actividad guarda los IDs de sus dependencias como
texto, pero para trabajar rápido conviene tener todo en números. Por eso se
arma un `unordered_map` que traduce cada ID a su posición en la lista. Con eso
se construyen dos cosas: para cada actividad, quiénes dependen de ella
(`dependientes`), y cuántas dependencias le faltan por cumplir (`pendientes`).
De paso se revisa que no haya IDs repetidos ni dependencias que apunten a
actividades que no existen.

Antes de ejecutar nada, se simula el orden topológico completo (algoritmo de
Kahn) solo para detectar si el plan tiene un ciclo. Si una actividad nunca
llega a tener 0 dependencias pendientes, es porque está encerrada en un ciclo
junto a otras, y ahí se informa cuáles son y se corta.

**Ejecutar con procesos.** Las actividades listas (sin dependencias) se van
sacando de una cola y cada una se lanza con `fork()`. El padre nunca deja que
haya más de K hijos corriendo a la vez: si ya llegó al límite, deja de lanzar
y espera a que alguno termine con `waitpid`, que bloquea sin gastar CPU (nada
de ciclos revisando a cada rato si ya terminó).

**Mensajes entre procesos.** Cuando un hijo termina, tiene que avisarle al
padre. Para eso se usan pipes con nombre (FIFOs), uno por actividad, creados
con el PID del hijo en el nombre del archivo (`/tmp/fifo_<pid>_<id>`) para que
no choquen si alguien corre el programa dos veces al mismo tiempo. El hijo
escribe "OK" o "FALLO" y cierra; el padre lo lee apenas confirma con
`waitpid` que el hijo ya terminó.

**Si algo falla.** El padre no se queda solo con el mensaje del pipe, también
mira el código de salida real del proceso (`WIFEXITED`, `WEXITSTATUS`). Si una
actividad falla, se marca a todos los que dependían de ella (y a los que
dependían de esos, y así en cadena) para que nunca se ejecuten. El resto del
plan, si no tiene nada que ver con la rama que falló, sigue como si nada.

**Ctrl+C (SIGINT).** Se instala un manejador con `sigaction` que solo levanta
una bandera (`interrumpido`), siguiendo la práctica de no hacer trabajo
pesado dentro del handler. El programa revisa esa bandera entre cada
iteración y, al detectarla, mata con `SIGKILL` a todos los procesos hijos que
sigan activos, espera a que todos terminen, limpia los FIFOs pendientes y
corta la ejecución.

## Prueba de carga (10000 actividades)

Para generar un plan grande y probar que el programa aguanta sin caerse ni
colgarse:

```bash
python3 generar_plan_10mil.py
./planificador plan_10mil.txt 50
```

El script arma un DAG de 10000 actividades donde cada una solo puede
depender de actividades con un ID menor al suyo, lo que garantiza que el
grafo nunca tenga ciclos sin necesidad de revisarlo aparte. Se probó con
distintos valores de K, y el programa procesa las 10000 actividades
completas sin caídas ni cuelgues.

## Por qué tomamos estas decisiones

Usamos C++ y no C porque `std::string` y `std::vector` ahorran mucho trabajo
de manejo de memoria que en C habría que hacer a mano.

El ID lo guardamos como string y no como número porque el enunciado dice que
es alfanumérico, así que podría venir como "A1" y no solo como un entero.

Para esperar a los hijos usamos `waitpid` bloqueante en vez de meter un
`poll` o `select`. Hace exactamente lo que se pide (nada de busy-waiting) y es
bastante más simple de razonar y de explicar.

Los pipes los hicimos con nombre (`mkfifo`) en vez de pipes anónimos. La razón
principal es que así cada mensaje queda asociado a un archivo identificable
por actividad, lo que hizo más fácil debuggear cuando algo no llegaba. El
riesgo de este enfoque es que si dos ejecuciones corren a la vez en la misma
máquina podrían chocar los nombres, así que metimos el PID del hijo en el
nombre del archivo para evitarlo.

Lo de `FALLO_PROB` como variable de entorno fue para poder demostrar que el
aislamiento de errores funciona sin tener que inventar un formato especial
dentro de `plan.txt`. Por defecto el programa es determinista (nunca falla
nada), y solo si uno quiere probarlo activa la variable.

Mantuvimos la temática del asado/Fiestas Patrias en los mensajes de consola
(`"la comida se pone al fuego"`, `"no hay carbon"`, `"llegaron los pacos"`,
etc.) porque el enunciado mismo plantea el programa así, con el señor Loyola
organizando su celebración. Nos pareció una forma de mantener consistencia
con el enunciado y darle una identidad más clara al proyecto, sin que afecte
en nada la lógica ni el cumplimiento de los requisitos técnicos.

## Estado del proyecto

Requisitos de la rúbrica que están implementados:

- Parseo de plan.txt
- Modelado del DAG y detección de ciclos
- Creación de procesos por actividad
- Control de concurrencia K
- Paso de mensajes con pipes
- Aislamiento de errores
- Manejo de Ctrl+C (SIGINT)
- Prueba de estrés con plan de 10000 actividades (ver `generar_plan_10mil.py`)
