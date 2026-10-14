/*
 * SO-T3-DanielMonterrosa
 * box.c - Programa Box (main): loop de solicitudes, informe final,
 *         timeout de caos planetario.
 *
 * Uso: ./bin/box [-t timeout_seg] [-m 1|2] [--sim-min X] [--sim-max Y]
 *   -t: segundos para caos planetario (default 300, mínimo sugerido 300)
 *   -m: 1 = preguntar dificultad al usuario, 2 = aleatoria (default 2)
 *
 * Flujo:
 *  1. ipc_init (shm+sem), srand, alarma de caos.
 *  2. Loop: pide tipo A/B/C hasta despedida (chao/adios/bye/salir).
 *  3. Por solicitud: clock_gettime, fork Meeseeks N=1,i=1 via pipes,
 *     espera resultado por pipe, clock_gettime (resta sim para tiempo real).
 *  4. Informe final + ipc_cleanup + matar hijo pendiente si hubo caos.
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
#include "common.h"
#include "meeseeks.h"
#include "caso_a.h"
#include "ipc.h"
#include "utils.h"

config_t g_config = {
    .caos_timeout_seg = 300,
    .dificultad_modo = 2,
    .sim_min_seg = 0.5f,
    .sim_max_seg = 5.0f
};

static volatile sig_atomic_t g_caos = 0;
static volatile pid_t g_hijo_actual = -1;

static void on_caos(int sig) {
    (void)sig;
    g_caos = 1;
    /* Matar subárbol completo del Meeseeks inicial (es líder de grupo) */
    if (g_hijo_actual > 0) {
        kill(-g_hijo_actual, SIGTERM);
    }
}

static void on_int(int sig) {
    (void)sig;
    g_caos = 1;
    if (g_hijo_actual > 0) kill(-g_hijo_actual, SIGTERM);
}

static void uso(const char *prog) {
    printf("Uso: %s [-t timeout_seg] [-m 1|2] [--sim-min X] [--sim-max Y]\n", prog);
    printf("  -t N      caos planetario en N seg (default 300)\n");
    printf("  -m 1|2    1=preguntar dificultad, 2=aleatoria (default 2)\n");
}

static void imprimir_informe(solicitud_t *hist, int n) {
    printf("\n================ INFORME FINAL DEL BOX ================\n");
    printf("Tareas solicitadas: %d\n", n);
    int ok = 0, fail = 0;
    for (int k = 0; k < n; k++) {
        if (hist[k].exito) ok++; else fail++;
    }
    printf("Completadas: %d | No completadas: %d\n", ok, fail);
    printf("Meeseeks totales creados: %d | Nivel máximo: %d\n",
           ipc_contador_total(), ipc_contador_maxN());
    printf("--------------------------------------------------------\n");
    for (int k = 0; k < n; k++) {
        printf("#%d [%c] '%s'\n", hist[k].id, (char)hist[k].tipo, hist[k].texto);
        if (hist[k].tipo == REQ_TEXTO)
            printf("    dificultad=%.2f\n", hist[k].dificultad);
        printf("    resultado=%s | wall=%.3fs sim=%.3fs real=%.3fs (sin sim)\n",
               hist[k].exito ? "ÉXITO" : "FRACASO",
               hist[k].wall_seg, hist[k].sim_seg, hist[k].duracion_seg);
        printf("    meeseeks=%d", hist[k].num_meeseeks);
        if (hist[k].exito)
            printf(" | ganador pid=%d ppid=%d N=%d i=%d",
                   (int)hist[k].meeseeks_ganador_pid,
                   (int)hist[k].meeseeks_ganador_ppid,
                   hist[k].ganador_N, hist[k].ganador_i);
        printf("\n    detalle: %s\n", hist[k].detalle);
    }
    printf("========================================================\n");
}

int main(int argc, char *argv[]) {
    /* ---- argv ---- */
    for (int a = 1; a < argc; a++) {
        if (strcmp(argv[a], "-t") == 0 && a + 1 < argc) {
            g_config.caos_timeout_seg = atoi(argv[++a]);
        } else if (strcmp(argv[a], "-m") == 0 && a + 1 < argc) {
            g_config.dificultad_modo = atoi(argv[++a]);
            if (g_config.dificultad_modo != 1) g_config.dificultad_modo = 2;
        } else if (strcmp(argv[a], "--sim-min") == 0 && a + 1 < argc) {
            g_config.sim_min_seg = (float)atof(argv[++a]);
        } else if (strcmp(argv[a], "--sim-max") == 0 && a + 1 < argc) {
            g_config.sim_max_seg = (float)atof(argv[++a]);
        } else if (strcmp(argv[a], "-h") == 0 || strcmp(argv[a], "--help") == 0) {
            uso(argv[0]);
            return 0;
        } else {
            printf("Opción desconocida: %s\n", argv[a]);
            uso(argv[0]);
            return 1;
        }
    }
    if (g_config.caos_timeout_seg < 10) {
        printf("Aviso: timeout muy bajo (%d s). Se recomienda >= 300.\n",
               g_config.caos_timeout_seg);
    }

    srand((unsigned)(getpid() + time(NULL)));
    setvbuf(stdout, NULL, _IOLBF, 0); /* línea a línea: evita duplicados tras fork */

    if (ipc_init() < 0) {
        fprintf(stderr, "Error inicializando IPC\n");
        return 1;
    }

    /* ---- caos planetario: alarma global ---- */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_caos;
    sigaction(SIGALRM, &sa, NULL);
    sa.sa_handler = on_int;
    sigaction(SIGINT, &sa, NULL);
    alarm((unsigned)g_config.caos_timeout_seg);

    printf("=== BOX Mr. Meeseeks ===\n");
    printf("Pida solicitudes. Tipos: A=texto, B=cálculo, C=programa.\n");
    printf("Para salir escriba: chao / adios / bye / salir\n");
    printf("Caos planetario en %d s. Dificultad modo %d. Sim [%.1f, %.1f]s.\n\n",
           g_config.caos_timeout_seg, g_config.dificultad_modo,
           g_config.sim_min_seg, g_config.sim_max_seg);

    solicitud_t hist[HIST_MAX];
    int n_hist = 0;
    int next_id = 1;
    char linea[600];

    while (!g_caos) {
        printf("\n--- Solicitud #%d ---\n", next_id);
        printf("Tipo [A/B/C o salir]: ");
        fflush(stdout);
        if (!fgets(linea, sizeof(linea), stdin)) break;
        trim_newline(linea);
        if (linea[0] == '\0') continue;
        if (es_despedida(linea)) break;

        req_tipo_t tipo;
        char c = linea[0];
        if (c == 'a' || c == 'A') tipo = REQ_TEXTO;
        else if (c == 'b' || c == 'B') tipo = REQ_CALCULO;
        else if (c == 'c' || c == 'C') tipo = REQ_PROGRAMA;
        else {
            printf("Tipo inválido. Use A, B, C o despedida.\n");
            continue;
        }

        if (tipo == REQ_TEXTO)
            printf("Solicitud textual (ej. 'Quiero un café francés'): ");
        else if (tipo == REQ_CALCULO)
            printf("Expresión (ej. '2 + 3 * 4', '5 > 3 && 1'): ");
        else
            printf("Comando externo (ej. 'ls -l', 'sleep 2'): ");
        fflush(stdout);
        if (!fgets(linea, sizeof(linea), stdin)) break;
        trim_newline(linea);
        if (es_despedida(linea)) break;
        if (linea[0] == '\0') { printf("Solicitud vacía, intente de nuevo.\n"); continue; }

        if (n_hist >= HIST_MAX) {
            printf("Historial lleno (%d). Cerrando Box.\n", HIST_MAX);
            break;
        }

        solicitud_t sol;
        memset(&sol, 0, sizeof(sol));
        sol.id = next_id++;
        sol.tipo = tipo;
        strncpy(sol.texto, linea, sizeof(sol.texto) - 1);
        sol.dificultad = -1;

        if (tipo == REQ_TEXTO) {
            sol.dificultad = caso_a_obtener_dificultad(sol.texto);
            printf("Box: dificultad asignada = %.2f\n", sol.dificultad);
        }

        /* ---- crear Meeseeks inicial N=1,i=1 via fork+pipe ---- */
        clock_gettime(CLOCK_MONOTONIC, &sol.t_inicio);
        int total_antes = ipc_contador_total();
        fflush(stdout); /* vaciar buffer antes del fork (evita líneas duplicadas) */

        int fds[2] = {-1, -1};
        pid_t hijo = meeseeks_crear_inicial(&sol, fds);
        if (hijo < 0) {
            printf("Box: no pude crear al Meeseeks (fork falló).\n");
            continue;
        }
        g_hijo_actual = hijo;
        printf("Box: creé a Mr. Meeseeks pid=%d para '%s'\n", (int)hijo, sol.texto);

        /* Enviar solicitud por mensaje privado via pipe (no var global) */
        if (ipc_pipe_enviar(fds[0], &sol, sizeof(sol)) < 0)
            printf("Box: aviso, fallo enviando solicitud por pipe.\n");
        close(fds[0]);

        /* Esperar resultado por pipe + wait */
        meeseeks_result_t res;
        memset(&res, 0, sizeof(res));
        int got = (ipc_pipe_recibir(fds[1], &res, sizeof(res)) == 0);
        close(fds[1]);

        int st = 0;
        while (waitpid(hijo, &st, 0) < 0 && errno == EINTR) {
            if (g_caos) break;
        }
        g_hijo_actual = -1;
        clock_gettime(CLOCK_MONOTONIC, &sol.t_fin);

        sol.wall_seg = timespec_diff_seg(sol.t_inicio, sol.t_fin);
        sol.sim_seg = got ? res.sim_time : 0.0;
        sol.duracion_seg = sol.wall_seg - sol.sim_seg; /* real sin simulación */
        if (sol.duracion_seg < 0) sol.duracion_seg = 0;
        sol.num_meeseeks = ipc_contador_total() - total_antes;
        if (sol.num_meeseeks < 1) sol.num_meeseeks = 1;

        if (g_caos) {
            sol.exito = 0;
            snprintf(sol.detalle, sizeof(sol.detalle),
                     "interrumpida por caos planetario");
            printf("\n¡CAOS PLANETARIO! Tiempo máximo (%d s) agotado.\n",
                   g_config.caos_timeout_seg);
            hist[n_hist++] = sol;
            break;
        }

        if (got) {
            sol.exito = res.exito;
            sol.meeseeks_ganador_pid = res.ganador_pid;
            sol.meeseeks_ganador_ppid = res.ganador_ppid;
            sol.ganador_N = res.ganador_N;
            sol.ganador_i = res.ganador_i;
            strncpy(sol.detalle, res.detalle, sizeof(sol.detalle) - 1);
        } else {
            sol.exito = (WIFEXITED(st) && WEXITSTATUS(st) == 0);
            snprintf(sol.detalle, sizeof(sol.detalle),
                     "sin mensaje por pipe (exit %d)", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
            sol.meeseeks_ganador_pid = hijo;
            sol.ganador_N = 1; sol.ganador_i = 1;
        }

        if (sol.exito) {
            printf("Box al usuario: ¡Solicitud #%d COMPLETADA! %s\n", sol.id, sol.detalle);
            printf("  Resuelta por Meeseeks pid=%d ppid=%d N=%d i=%d\n",
                   (int)sol.meeseeks_ganador_pid, (int)sol.meeseeks_ganador_ppid,
                   sol.ganador_N, sol.ganador_i);
        } else {
            printf("Box al usuario: Solicitud #%d NO se pudo completar. %s\n",
                   sol.id, sol.detalle);
        }
        printf("  Tiempos: wall=%.3fs sim=%.3fs real=%.3fs | Meeseeks usados: %d\n",
               sol.wall_seg, sol.sim_seg, sol.duracion_seg, sol.num_meeseeks);

        hist[n_hist++] = sol;
    }

    alarm(0);
    /* Si quedó hijo vivo (caos), asegurar terminación */
    if (g_hijo_actual > 0) {
        kill(-g_hijo_actual, SIGTERM);
        sleep(1);
        kill(-g_hijo_actual, SIGKILL);
        while (waitpid(-1, NULL, WNOHANG) > 0) {}
    }

    imprimir_informe(hist, n_hist);
    ipc_cleanup();
    printf("Box terminado. ¡Adiós!\n");
    return 0;
}
