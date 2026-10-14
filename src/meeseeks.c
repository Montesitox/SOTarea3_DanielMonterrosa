/*
 * SO-T3-DanielMonterrosa
 * meeseeks.c - fork, saludo/despedida, delegación recursiva Caso A.
 *
 * Todos los hijos ejecutan el mismo código (requisito tarea).
 * Mensajes privados via pipe; contadores via shm+sem (ipc.c).
 * Tiempos reales con clock_gettime; sleeps solo simulan trabajo.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "meeseeks.h"
#include "common.h"
#include "caso_a.h"
#include "caso_b.h"
#include "caso_c.h"
#include "ipc.h"
#include "utils.h"

/* Para despedirse si el padre nos mata tras éxito de un hermano */
static int g_N = 1, g_i = 1;
static void on_sigterm(int sig) {
    (void)sig;
    pid_t pid = getpid(), ppid = getppid();
    /* write es async-safe; usamos dprintf para el adiós */
    dprintf(STDOUT_FILENO,
        "[pid=%d ppid=%d] Meeseeks (N=%d,i=%d) recibe orden de terminar, se despide. ¡Adiós!\n",
        (int)pid, (int)ppid, g_N, g_i);
    _exit(2);
}

void meeseeks_saludar(pid_t pid, pid_t ppid, int N, int i) {
    printf("\n[pid=%d ppid=%d] Hi, I'm Mr. Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n",
           (int)pid, (int)ppid, (int)pid, (int)ppid, N, i);
    fflush(stdout);
}

void meeseeks_despedirse(pid_t pid, pid_t ppid, int N, int i, const char *motivo) {
    printf("[pid=%d ppid=%d] Meeseeks (N=%d,i=%d) %s. ¡Adiós! *desaparece*\n",
           (int)pid, (int)ppid, N, i, motivo ? motivo : "termina");
    fflush(stdout);
}

/* Nodo recursivo: NO retorna, termina con _exit(code).
 * fd_down: lectura solicitud (propio), fd_up: escritura resultado al padre. */
static void meeseeks_nodo(solicitud_t sol, int N, int i, int fd_down, int fd_up) {
    alarm(0); /* el timeout de caos solo lo maneja el Box */
    signal(SIGTERM, on_sigterm);
    signal(SIGPIPE, SIG_IGN);
    g_N = N; g_i = i;
    srand((unsigned)(getpid() * 7919u + (unsigned)time(NULL)));

    pid_t pid = getpid(), ppid = getppid();

    /* Leer solicitud por mensaje privado via pipe (requisito) */
    if (fd_down >= 0) {
        solicitud_t tmp;
        if (ipc_pipe_recibir(fd_down, &tmp, sizeof(tmp)) == 0) {
            sol = tmp;
        }
        close(fd_down);
    }
    meeseeks_saludar(pid, ppid, N, i);
    ipc_contador_nuevo_meeseeks(N); /* shm+sem: recurso compartido */

    meeseeks_result_t res;
    memset(&res, 0, sizeof(res));
    res.exito = 0;
    res.ganador_pid = pid;
    res.ganador_ppid = ppid;
    res.ganador_N = N;
    res.ganador_i = i;
    res.sim_time = 0.0;

    /* ============ Caso B: cálculo, 1 solo Meeseeks ============ */
    if (sol.tipo == REQ_CALCULO) {
        char out[256];
        log_msg("Caso B: calculando '%s' (N=%d,i=%d)...", sol.texto, N, i);
        int rc = caso_b_evaluar(sol.texto, out, sizeof(out));
        if (rc == 0) {
            res.exito = 1;
            snprintf(res.detalle, sizeof(res.detalle), "cálculo OK: %s = %s", sol.texto, out);
        } else {
            res.exito = 0;
            snprintf(res.detalle, sizeof(res.detalle), "%s", out);
            log_msg("No pude resolver el cálculo: %s", out);
        }
        res.sim_time = 0.0;
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i,
            res.exito ? "solicitud completada, agradece haberle servido" : "falló el cálculo");
        _exit(res.exito ? 0 : 1);
    }

    /* ============ Caso C: programa externo ============ */
    if (sol.tipo == REQ_PROGRAMA) {
        log_msg("Caso C: ejecutando '%s' (N=%d,i=%d)...", sol.texto, N, i);
        int code = -1;
        double dur = 0;
        int rc = caso_c_ejecutar(sol.texto, &code, &dur);
        if (rc == 0 && code == 0) {
            res.exito = 1;
            snprintf(res.detalle, sizeof(res.detalle),
                     "programa OK exit=0 en %.3fs: %s", dur, sol.texto);
        } else {
            res.exito = 0;
            snprintf(res.detalle, sizeof(res.detalle),
                     "programa falló exit=%d en %.3fs: %s", code, dur, sol.texto);
            log_msg("Programa terminó con errores (exit %d). Reporto fracaso.", code);
        }
        res.sim_time = 0.0; /* ejecución externa es tiempo real, no simulado */
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i,
            res.exito ? "solicitud completada, agradece haberle servido" : "el programa falló");
        _exit(res.exito ? 0 : 1);
    }

    /* ============ Caso A: textual con dificultad ============ */
    float dif = sol.dificultad;
    double sim = caso_a_tiempo_simulacion();
    log_msg("Caso A: '%s' dificultad=%.2f, trabajando %.2fs (N=%d,i=%d)...",
            sol.texto, dif, sim, N, i);
    {
        struct timespec ts;
        ts.tv_sec = (time_t)sim;
        ts.tv_nsec = (long)((sim - ts.tv_sec) * 1e9);
        nanosleep(&ts, NULL); /* simulación; Box lo restará del tiempo real */
    }
    res.sim_time = sim;

    /* Dificultad 0: imposible, jamás resuelve (pero respeta contingencia) */
    int puedo = caso_a_intentar_resolver(dif);
    if (puedo) {
        res.exito = 1;
        snprintf(res.detalle, sizeof(res.detalle),
                 "solicitud '%s' completada por Meeseeks (%d,%d,%d,%d)",
                 sol.texto, (int)pid, (int)ppid, N, i);
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i, "solicitud completada, agradece haberle servido");
        _exit(0);
    }

    int n_hijos = caso_a_num_hijos(dif);
    if (n_hijos == 0) {
        /* 85.01-100 pero perdió la apuesta (10%): fracaso sin hijos */
        res.exito = 0;
        snprintf(res.detalle, sizeof(res.detalle),
                 "aun siendo fácil (%.2f) fallé", dif);
        log_msg("No pude resolverlo yo solo. Reporto fracaso al Box.");
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i, "no pudo resolver la tarea");
        _exit(1);
    }

    /* Contingencia anti fork-bomb (documentada en docs/diseno.md) */
    if (N >= MEESEEKS_MAX_NIVELES ||
        ipc_contador_total() + n_hijos > MEESEEKS_MAX_TOTAL) {
        res.exito = 0;
        snprintf(res.detalle, sizeof(res.detalle),
                 "límite alcanzado (N=%d, total=%d): tarea imposible, evito colapso",
                 N, ipc_contador_total());
        log_msg("¡CAOS EVITADO! Llego al límite (N=%d). No creo más Meeseeks.", N);
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i, "declara tarea imposible por seguridad");
        _exit(1);
    }

    log_msg("¡Necesito ayuda! Creando %d Meeseeks hijos (N=%d)...", n_hijos, N + 1);
    float dif_hija = caso_a_diluir(dif, n_hijos);
    log_msg("Aging: dificultad %.2f -> %.2f para los hijos", dif, dif_hija);

    pid_t hijos[MEESEEKS_MAX_HIJOS];
    int up_read[MEESEEKS_MAX_HIJOS];
    int alive[MEESEEKS_MAX_HIJOS];
    meeseeks_result_t res_h[MEESEEKS_MAX_HIJOS];
    int got_h[MEESEEKS_MAX_HIJOS];
    for (int k = 0; k < n_hijos; k++) {
        hijos[k] = -1; up_read[k] = -1; alive[k] = 0;
        got_h[k] = 0;
        memset(&res_h[k], 0, sizeof(res_h[k]));
    }

    for (int k = 0; k < n_hijos; k++) {
        int dpipe[2], upipe[2];
        if (ipc_pipe_crear(dpipe) < 0 || ipc_pipe_crear(upipe) < 0) {
            log_msg("Error creando pipes para hijo %d", k + 1);
            continue;
        }
        fflush(stdout);
        pid_t c = fork();
        if (c < 0) {
            log_msg("fork falló para hijo %d: %s", k + 1, strerror(errno));
            close(dpipe[0]); close(dpipe[1]);
            close(upipe[0]); close(upipe[1]);
            continue;
        }
        if (c == 0) {
            /* Hijo: cierra extremos del padre y ejecuta mismo código */
            close(dpipe[1]);
            close(upipe[0]);
            solicitud_t sub = sol;
            sub.dificultad = dif_hija;
            /* i: 1..n según especificación (N+1 respecto al padre) */
            meeseeks_nodo(sub, N + 1, k + 1, dpipe[0], upipe[1]);
            _exit(1); /* nunca llega */
        }
        /* Padre: cierra extremos del hijo */
        close(dpipe[0]);
        close(upipe[1]);
        hijos[k] = c;
        up_read[k] = upipe[0];
        alive[k] = 1;
        /* Enviar sub-solicitud por mensaje privado via pipe */
        solicitud_t sub = sol;
        sub.dificultad = dif_hija;
        if (ipc_pipe_enviar(dpipe[1], &sub, sizeof(sub)) < 0)
            log_msg("Aviso: no pude enviar solicitud al hijo %d", k + 1);
        close(dpipe[1]);
    }

    /* Esperar hijos; si uno tiene éxito, ordenar a hermanos terminar */
    int pendientes = 0;
    for (int k = 0; k < n_hijos; k++) if (hijos[k] > 0) pendientes++;
    int ganador_idx = -1;

    while (pendientes > 0) {
        int st = 0;
        pid_t w = waitpid(-1, &st, WNOHANG);
        if (w > 0) {
            for (int k = 0; k < n_hijos; k++) {
                if (hijos[k] == w) {
                    alive[k] = 0;
                    pendientes--;
                    int ok = (WIFEXITED(st) && WEXITSTATUS(st) == 0);
                    /* Leer resultado del hijo por su pipe */
                    if (ipc_pipe_recibir(up_read[k], &res_h[k], sizeof(res_h[k])) == 0)
                        got_h[k] = 1;
                    else {
                        got_h[k] = 0;
                        res_h[k].exito = ok ? 1 : 0;
                        res_h[k].sim_time = 0;
                    }
                    if (ok && ganador_idx < 0) {
                        ganador_idx = k;
                        /* Avisar a hermanos que terminen (requisito) */
                        for (int j = 0; j < n_hijos; j++) {
                            if (alive[j] && hijos[j] > 0) {
                                log_msg("Comunicando al hermano (i=%d,pid=%d) que termine...",
                                        j + 1, (int)hijos[j]);
                                kill(hijos[j], SIGTERM);
                            }
                        }
                    }
                    close(up_read[k]);
                    up_read[k] = -1;
                    break;
                }
            }
        } else if (w == 0) {
            struct timespec t = {0, 50000000L}; /* 50ms */
            nanosleep(&t, NULL);
        } else {
            if (errno == EINTR) continue;
            if (errno == ECHILD) break;
            struct timespec t = {0, 50000000L};
            nanosleep(&t, NULL);
        }
    }
    /* Cierre defensivo de pipes restantes */
    for (int k = 0; k < n_hijos; k++)
        if (up_read[k] >= 0) { close(up_read[k]); up_read[k] = -1; }

    /* Agregar tiempos simulados de hijos */
    for (int k = 0; k < n_hijos; k++)
        if (got_h[k]) res.sim_time += res_h[k].sim_time;

    if (ganador_idx >= 0 && got_h[ganador_idx]) {
        /* Propagar éxito hacia arriba preservando al ganador original */
        res = res_h[ganador_idx];
        /* sim_time ya incluye el propio + hijos? res_h trae subárbol;
         * sumar el propio sim: */
        res.sim_time += sim;
        log_msg("Hijo (N=%d,i=%d) resolvió. Propago éxito al padre y me despido...",
                res.ganador_N, res.ganador_i);
        if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
        meeseeks_despedirse(pid, ppid, N, i, "propaga éxito hacia arriba");
        _exit(0);
    }
    /* Todos fallaron */
    res.exito = 0;
    res.ganador_pid = pid;
    res.ganador_ppid = ppid;
    res.ganador_N = N;
    res.ganador_i = i;
    snprintf(res.detalle, sizeof(res.detalle),
             "ni con %d ayudantes (N=%d) se logró '%.100s'", n_hijos, N, sol.texto);
    log_msg("Ningún hijo lo logró. Reporto fracaso hacia arriba...");
    if (fd_up >= 0) { ipc_pipe_enviar(fd_up, &res, sizeof(res)); close(fd_up); }
    meeseeks_despedirse(pid, ppid, N, i, "no se pudo cumplir la misión");
    _exit(1);
}

void meeseeks_ejecutar(solicitud_t sol, int N, int i, int pipe_lectura_fd) {
    /* Si nos dieron pipe de resultado aparte no lo conocemos aquí;
     * esta función es para el primer Meeseeks sin pipe de subida:
     * crea su propio flujo con fd_up = -1 (Box usa exit code + pipe). */
    (void)pipe_lectura_fd;
    meeseeks_nodo(sol, N, i, pipe_lectura_fd, -1);
}

pid_t meeseeks_crear_inicial(const solicitud_t *sol, int fds_padre[2]) {
    int dpipe[2], upipe[2];
    if (ipc_pipe_crear(dpipe) < 0 || ipc_pipe_crear(upipe) < 0) return -1;
    fflush(stdout); /* evita duplicar buffer del Box en el hijo */
    pid_t c = fork();
    if (c < 0) {
        close(dpipe[0]); close(dpipe[1]);
        close(upipe[0]); close(upipe[1]);
        return -1;
    }
    if (c == 0) {
        /* Hijo Meeseeks N=1,i=1: líder de grupo para que el Box
         * pueda matar el subárbol completo con kill(-pid) en caos */
        setpgid(0, 0);
        close(dpipe[1]);
        close(upipe[0]);
        meeseeks_nodo(*sol, 1, 1, dpipe[0], upipe[1]);
        _exit(1); /* nunca */
    }
    /* Padre Box */
    close(dpipe[0]);
    close(upipe[1]);
    fds_padre[0] = dpipe[1]; /* Box escribe solicitud */
    fds_padre[1] = upipe[0]; /* Box lee resultado */
    return c;
}
