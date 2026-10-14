# SO-T3-DanielMonterrosa — Box Mr. Meeseeks

Tarea #3 Sistemas Operativos: Box de Mr. Meeseeks en C sobre Linux.

## Estructura
```
.
├── Instrucciones.txt
├── Makefile
├── bin/                  # binarios (ignorado por git)
├── include/              # headers
│   ├── common.h          # structs, config global, tipos solicitud
│   ├── meeseeks.h        # ciclo de vida Meeseeks (fork, N, i)
│   ├── caso_a.h          # solicitudes textuales (dificultad 0-100)
│   ├── caso_b.h          # cálculos aritméticos/lógicos
│   ├── caso_c.h          # ejecución programa externo
│   ├── ipc.h             # pipes, shm, semáforos
│   └── utils.h           # tiempos, random, logging pid/ppid
├── src/
│   ├── box.c             # main: loop Box, informe final, caos planetario
│   ├── meeseeks.c
│   ├── caso_a_texto.c
│   ├── caso_b_calculo.c
│   ├── caso_c_externo.c
│   ├── ipc.c
│   └── utils.c
├── docs/
│   └── diseno.md         # diagramas, contingencia dificultad 0
└── tests/
    └── (pruebas por caso A/B/C)
```

## Compilar (Ubuntu)
```bash
make
./bin/box
```

## Reglas clave de la tarea
- Un único programa; hijos con `fork()` ejecutan el mismo código.
- Mensaje bienvenida: `Hi, I'm Mr. Meeseeks! Look at Meeeee. (pid, ppid, N, i)`.
- Pipes para mensajes privados, semáforos + shm para contadores N/i.
- Informe final al cerrar Box: # tareas, duración c/u, éxito/fracaso, # Meeseks por tarea.
- Timeout configurable caos planetario (base 5 min).
