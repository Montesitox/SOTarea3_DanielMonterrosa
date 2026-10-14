# Diseño Box Mr. Meeseeks (SO-T3-DanielMonterrosa)

## 1. Organización del código
- `src/box.c` (padre/Box): parse argv (`-t -m --sim-min --sim-max`), `ipc_init`,
  `alarm(caos)`, loop A/B/C, `fork` inicial, pipes solicitud/resultado,
  `clock_gettime`, informe final. Señales: `SIGALRM`->caos + `kill(-pid)`.
- `src/meeseeks.c`: `fork()`, saludo `(pid,ppid,N,i)`, nodo recursivo
  `meeseeks_nodo()` (mismo binario en todos los hijos), delegación Caso A,
  `SIGTERM`->despedida, `setpgid` en Meeseeks inicial (grupo para matar subárbol).
- `src/caso_a_texto.c`: dificultad, tiempo sim, apuesta `p=dif/100`, `aging`.
- `src/caso_b_calculo.c`: parser recursivo `+ - * / % ^ ( ) > < == != && || !`.
- `src/caso_c_externo.c`: `fork+execl(/bin/sh -c)+waitpid`, `WEXITSTATUS`.
- `src/ipc.c`: `shm_open+ mmap` + `sem_open` (contadores), `pipe` + envío exacto.
- `src/utils.c`: `timespec_diff`, `rand_float`, `log_msg [pid/ppid]`, despedidas.

## 2. Comunicación entre procesos
```
Box --pipe(solicitud_t)--> Meeseeks N=1,i=1
  Meeseeks --pipe(solicitud hija, dif diluida)--> hijos N+1 (2 o 5)
  Hijo ganador --pipe(meeseeks_result_t)--> padre
  Padre --kill(SIGTERM)--> hermanos (terminen y despidan)
  Padre --pipe(resultado ganador)--> su padre ... recursivo hasta Box
  Todos imprimen [pid/ppid] + saludo/despedida antes de _exit
```
- Prohibido var global para mensajes: solo `solicitud_t`/`meeseeks_result_t` por pipes.
- Permitido shm global: `total_meeseeks`, `max_N` con `sem_wait/post`.

## 3. Plan de contingencia dificultad 0 (imposible)
Riesgo: dif 0 genera 5 hijos por nivel -> 5^6 = 15625 procesos (fork-bomb).
Acciones (`MEESEEKS_MAX_NIVELES=6`, `MEESEEKS_MAX_TOTAL=200` en `common.h`):
1. `caso_a_intentar_resolver(0)` siempre retorna 0 (nunca hay éxito).
2. `caso_a_diluir(0)` retorna 0 (lo imposible no se ablanda).
3. Antes de `fork`, si `N >= MAX_NIVELES` o `total+hijos > MAX_TOTAL`:
   no forkea, escribe fracaso "límite alcanzado, evito colapso", `_exit(1)`.
4. El fracaso se propaga hacia arriba con despedidas en cadena.
5. Box reporta FRACASO + nº Meeseeks acotado (~195 con 5 hijos y 6 niveles).

## 4. Tiempos (requisito: excluir simulación)
- Box: `clock_gettime(MONOTONIC)` antes de `fork`, después de `wait` => `wall`.
- Meeseeks Caso A: `nanosleep(sim)` y acumula `sim_time` (+ hijos).
- Box: `real = wall - sim_time` (>=0). Casos B/C: `sim=0`, `real=wall`
  (C mide además duración interna del `exec`).

## 5. Casos B y C
- B gramática: `or := and ("||" and)*`, `and := eq ("&&" eq)*`,
  `eq := rel (("=="|"!=") rel)*`, `rel := add ((>=|<=|>|<) add)*`,
  `add := mul ((+|-) mul)*`, `mul := pow ((*|/|%) pow)*`,
  `pow := unary ("^" unary)?`, `unary := (!|-|+) unary | número | "(" expr ")"`.
  Ej: `2 + 3 * 4`=14, `(2+3)*4`=20, `10 % 3`=1, `2 ^ 8`=256,
  `5 > 3 && 1`=1, `!(0 || 0)`=1. Errores: `división por cero`, `token inválido`,
  `se esperaba ')'`. Un solo Meeseeks, sin hijos.
- C: `execl("/bin/sh","sh","-c",cmd)`; éxito = `exit 0`.
  Ej: `ls /tmp`->0 éxito, `ls /noexiste`->2 fracaso, `false`->1, `sleep 1`->0.
  Solo éxito/fracaso para el reporte.

## 6. Experimentos 0x02 (pendiente de corridas)
- 3 corridas por caso A (fácil ~90, media ~60, imposible 0 con `-m 1`),
  B (4 expresiones), C (`true/false/ls`).
- Comparar `wall` vs `real` vs `sim`, nº Meeseeks, tasa éxito.
- Guardar salidas en `tests/salidas/` para el reporte PDF.
