/*
 * SO-T3-DanielMonterrosa
 * common.h - Definiciones compartidas Box / Meeseeks
 *
 * Contiene: tipos de solicitud (A/B/C), struct de resultado para
 * el informe final, y configuración global (caos planetario).
 */
#ifndef COMMON_H
#define COMMON_H

#include <time.h>

/* Tipos de solicitud */
typedef enum {
    REQ_TEXTO   = 'A',  /* Caso A: "Quiero..." -> simulación dificultad 0-100 */
    REQ_CALCULO = 'B',  /* Caso B: operación aritmética/lógica */
    REQ_PROGRAMA= 'C'   /* Caso C: ejecutar programa externo, ver exit code */
} req_tipo_t;

/* Una solicitud hecha al Box */
typedef struct {
    int id;                 /* consecutivo 1..N */
    req_tipo_t tipo;        /* A, B o C */
    char texto[512];        /* solicitud original del usuario */
    float dificultad;       /* 0-100, solo Caso A */
    struct timespec t_inicio;
    struct timespec t_fin;
    double duracion_seg;    /* tiempo real, sin sleeps de simulación */
    int exito;              /* 1 = completada, 0 = no */
    int num_meeseeks;       /* cuántos Meeseeks se crearon */
    pid_t meeseeks_ganador_pid;
    int ganador_N, ganador_i;
} solicitud_t;

/* Config global (seteable por usuario / argv) */
typedef struct {
    int caos_timeout_seg;   /* default 300 (5 min) */
    int dificultad_modo;    /* 1 = pregunta usuario, 2 = aleatoria */
    float sim_min_seg;      /* default 0.5 */
    float sim_max_seg;      /* default 5.0 */
} config_t;

extern config_t g_config;

#endif
