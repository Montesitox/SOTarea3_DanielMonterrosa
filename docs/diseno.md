# Diseño Box Mr. Meeseeks

## 1. Organización del código
- `box.c` (padre): loop + informe + alarma caos.
- `meeseeks.c`: `fork()`, saludo `(pid,ppid,N,i)`, delegación recursiva.
- `caso_a/b/c`: lógica por tipo de solicitud.
- `ipc.c`: pipes (solicitud privada) + shm/sem (contadores N/i).
- `utils.c`: `clock_gettime` para duración real, random, log con pid/ppid.

## 2. Comunicación entre procesos
```
Box --pipe(solicitud)--> Meeseeks N=1,i=1
  Meeseeks --pipe--> hijos N+1 (2 o 5 según dificultad)
  Hijo que resuelve --pipe--> padre --pipe--> hermanos (terminen)
  ... recursivo hasta Box, todos se despiden ...
```

## 3. Plan de contingencia dificultad 0 (imposible)
Para no colapsar con fork-bomb:
- `MAX_NIVELES` (ej. 6) y `MAX_MESEEKS_TOTAL` (ej. 200).
- Backoff: cada nivel aumenta sleep de simulación.
- Al llegar al límite: declarar tarea imposible, despedir en cadena,
  reportar fracaso en informe final. Documentar pruebas aquí.

## 4. Tiempos
- `clock_gettime(CLOCK_MONOTONIC)` antes/después del `fork+wait`.
- Excluir `usleep` de simulación del tiempo reportado
  (medir por separado o restar).

## 5. Casos B y C
- B: gramática soportada + ejemplos + errores (TODO).
- C: tabla exit codes 0/1/>1, ejemplo `ls`, `false`, script propio (TODO).

## 6. Experimentos 0x02
- 3 corridas por caso A/B/C con capturas.
- Comparar duración vs dificultad, nº Meeseeks, tasa éxito.
