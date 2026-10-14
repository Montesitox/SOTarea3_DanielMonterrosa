# SO-T3-DanielMonterrosa — Box Mr. Meeseeks

Tarea #3 Sistemas Operativos: Box de Mr. Meeseeks en C sobre Linux.

## Estructura
```
.
├── Instrucciones.txt
├── Makefile
├── bin/                  # binarios (ignorado por git)
├── include/              # headers
│   ├── common.h          # structs, config, límites MAX_NIVELES/TOTAL
│   ├── meeseeks.h        # ciclo de vida Meeseeks (fork, N, i)
│   ├── caso_a.h          # solicitudes textuales (dificultad 0-100)
│   ├── caso_b.h          # cálculos aritméticos/lógicos
│   ├── caso_c.h          # ejecución programa externo
│   ├── ipc.h             # pipes, shm, semáforos
│   └── utils.h           # tiempos, random, logging pid/ppid
├── src/
│   ├── box.c             # main: loop Box, informe final, caos planetario
│   ├── meeseeks.c        # nodo recursivo, delegación 2/5 hijos, aging
│   ├── caso_a_texto.c
│   ├── caso_b_calculo.c  # parser + - * / % ^ comparaciones lógicos
│   ├── caso_c_externo.c  # fork+exec+wait, éxito=exit 0
│   ├── ipc.c             # shm_open/mmap + sem_open + pipes exactos
│   └── utils.c
├── docs/
│   └── diseno.md         # diagramas, contingencia dificultad 0
└── tests/
    ├── prueba_a.sh       # 3 corridas caso A
    ├── prueba_b.sh       # 3 corridas caso B
    └── prueba_c.sh       # 3 corridas caso C
```

## Compilar y correr (Ubuntu)
```bash
make
./bin/box                  # modo aleatorio, caos 300s
./bin/box -m 1 -t 300      # preguntar dificultad, caos 5 min
./bin/box -t 60 --sim-min 0.5 --sim-max 1.5
```

## Uso interactivo
```
Tipo [A/B/C o salir]: A
Solicitud textual: Quiero un café francés
Dificultad ... (según -m 1|2)
...
Tipo [A/B/C o salir]: B
Expresión: 2 + 3 * 4
Tipo [A/B/C o salir]: C
Comando externo: ls -l /tmp
Tipo [A/B/C o salir]: chao   -> informe final
```

## Reglas clave implementadas
- Un único programa; hijos con `fork()` ejecutan el mismo código.
- Bienvenida: `Hi, I'm Mr. Meeseeks! Look at Meeeee. (pid, ppid, N, i)`.
- Pipes para mensajes privados, semáforos + shm para contadores N/i.
- Informe final: # tareas, wall/sim/real c/u, éxito/fracaso, # Meeseeks, ganador.
- Timeout configurable caos planetario (default 300s, `kill(-pid)` al subárbol).
- Contingencia dif 0: `MAX_NIVELES=6`, `MAX_TOTAL=200`, sin dilución.
