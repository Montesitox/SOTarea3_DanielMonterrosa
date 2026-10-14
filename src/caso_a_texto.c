/*
 * SO-T3-DanielMonterrosa
 * caso_a_texto.c - Dificultad, simulación y resolución probabilística.
 */
#include <stdlib.h>
#include "caso_a.h"
#include "utils.h"
#include "common.h"

extern config_t g_config;

float caso_a_obtener_dificultad(const char *solicitud) {
    (void)solicitud;
    /* TODO modo 1: pedir al usuario 0-100; modo 2: rand_float(0,100) */
    return rand_float(0.0f, 100.0f);
}

double caso_a_tiempo_simulacion(void) {
    return (double)rand_float(g_config.sim_min_seg, g_config.sim_max_seg);
}

int caso_a_intentar_resolver(float dificultad) {
    /* TODO: propuesta creativa: p = dificultad/100; rand < p => éxito.
     * Dificultad 0 => siempre falla. */
    if (dificultad <= 0.0f) return 0;
    return 1; /* esqueleto */
}

int caso_a_num_hijos(float dificultad) {
    if (dificultad > 85.0f) return 0;   /* 85.01-100: solo */
    if (dificultad > 45.0f) return 2;   /* 45.01-85: 2 hijos */
    return 5;                            /* 0-45: 5 hijos */
}

float caso_a_diluir(float dificultad, int num_hijos) {
    /* TODO aging: repartir dificultad entre hijos, ej. dificultad + delta */
    (void)num_hijos;
    return dificultad;
}
