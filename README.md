# ProcMan

ProcMan es un **manejador de procesos simple** implementado en C que permite crear, listar, terminar y monitorear procesos hijos desde una **shell interactiva**. Incluye manejo de señales y visualización de un árbol de procesos.

---

## Características

- Crear procesos con cualquier comando del sistema (`create <cmd> [args...]`).
- Listar procesos creados mostrando PID, comando, tiempo de ejecución y estado (`list`).
- Terminar procesos individualmente (`kill <pid> [force]`).
- Esperar a que todos los procesos hijos finalicen (`wait`).
- Visualizar árbol de procesos (`tree`).
- Manejo de señales:
  - `SIGINT` (Ctrl+C): termina todos los procesos hijos limpiamente.
  - `SIGCHLD`: evita procesos zombie.
- Shell interactiva con comandos `help` y `quit`.

---

## Requisitos

- Sistema operativo Linux (por el uso de `/proc` y `fork`/`exec`).
- Compilador GCC compatible con C99.

---

## Compilación

Compilar el proyecto:

```bash
make```

Esto generará el ejecutable procman.
Limpiar archivos objeto y ejecutable:

```bash
make clean
```

## Uso
```bash
./procman
```


### Ejemplos de comandos:
```
help
create sleep 5
list
kill 1234
wait
tree
quit
```

create <cmd> [args...] crea un proceso hijo.
list muestra los procesos activos y terminados.
kill <pid> termina un proceso por su PID.
wait espera a que todos los procesos hijos finalicen.
tree imprime el árbol de procesos a partir del procman.
quit sale de ProcMan

### Ejemplo de Salida:
```text=== Bienvenido a ProcMan ===
Comandos disponibles: help, create <cmd> [args], list, kill <pid>, wait, tree, quit
ProcMan> create sleep 5
Created process 1234
ProcMan> list
PID     COMMAND         RUNTIME     STATUS
-----   -------------   --------    ----------
1234    sleep 5         00:00:01    Running
ProcMan> wait
Waiting for all child processes...
Terminated process 1234
All processes handled.
ProcMan> tree
[5678] procman
  [1234] sleep
ProcMan> quit
Saliendo de ProcMan...
```


