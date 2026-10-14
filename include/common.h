/*
 * SO-T3-DanielMonterrosa
 * common.h - Definiciones compartidas Box / Meeseeks
 *
 * Contiene: tipos de solicitud (A/B/C), struct de resultado para
 * el informe final, y configuración global (caos planetario).
 *
 * Autor: Daniel Monterrosa - Sistemas Operativos Tarea #3 Mr. Meeseeks
 * Curso: Sistemas Operativos, Prof. M.Sc. Esteban Arias-Méndez
 */
#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <sys/types.h>

/* Tipos de solicitud */
typedef enum {
    REQ_TEXTO    = 'A',  /* Caso A: "Quiero..." -> simulación dificultad 0-100 */
    REQ_CALCULO  = 'B',  /* Caso B: operación aritmética/lógica */
    REQ_PROGRAMA = 'C'   /* Caso C: ejecutar programa externo, ver exit code */
} req_tipo_t;

/* Límites de contingencia (dificultad 0 = imposible, evita fork-bomb) */
#define MEESEEKS_MAX_NIVELES  6    /* profundidad máxima de delegación */
#define MEESEEKS_MAX_TOTAL    200  /* tope global de Meeseeks por solicitud */
#define MEESEEKS_MAX_HIJOS    5
#define HIST_MAX              100

/* Una solicitud hecha al Box */
typedef struct {
    int id;                 /* consecutivo 1..N */
    req_tipo_t tipo;        /* A, B o C */
    char texto[512];        /* solicitud original del usuario */
    float dificultad;       /* 0-100, solo Caso A (otros: -1) */
    struct timespec t_inicio;
    struct timespec t_fin;
    double wall_seg;        /* tiempo pared total fork+wait */
    double duracion_seg;    /* tiempo real = wall - sim (sin sleeps simulación) */
    double sim_seg;         /* tiempo simulado acumulado reportado por Meeseeks */
    int exito;              /* 1 = completada, 0 = no */
    int num_meeseeks;       /* cuántos Meeseeks se crearon */
    pid_t meeseeks_ganador_pid;
    pid_t meeseeks_ganador_ppid;
    int ganador_N, ganador_i;
    char detalle[256];
} solicitud_t;

/* Resultado que un Meeseeks envía a su padre por pipe (IPC, no var global) */
typedef struct {
    int exito;              /* 1 éxito, 0 fracaso */
    pid_t ganador_pid;
    pid_t ganador_ppid;
    int ganador_N;
    int ganador_i;
    double sim_time;        /* sleep simulado acumulado del subárbol */
    char detalle[256];
} meeseeks_result_t;

/* Config global (seteable por argv) */
typedef struct {
    int caos_timeout_seg;   /* default 300 (5 min) */
    int dificultad_modo;    /* 1 = pregunta usuario, 2 = aleatoria */
    float sim_min_seg;      /* default 0.5 */
    float sim_max_seg;      /* default 5.0 */
} config_t;

extern config_t g_config;

#endif
