/*
 * SO-T3-DanielMonterrosa
 * caso_a_texto.c - Dificultad, simulación y resolución probabilística.
 *
 * Idea creativa (estilo Meeseeks): cada Meeseeks apuesta a ciegas con
 * probabilidad p = dificultad/100 de "¡Ya sé cómo hacerlo!".
 *  - Si gana la apuesta -> resuelve solo, agradece y desaparece.
 *  - Si pierde -> grita "¡Necesito ayuda!" y abre la caja otra vez:
 *      0-45    -> 5 hijos (multitud del video), 45.01-85 -> 2 hijos.
 *  - Aging: la dificultad se diluye entre hijos para que la tarea se
 *    simplifique: hija = 100 - (100 - padre)/(hijos+1) + pequeño azar.
 *    Así converge a >85 en 1-2 niveles y evita explosión.
 *  - Dificultad 0 (imposible): nunca resuelve; contingencia con
 *    MEESEEKS_MAX_NIVELES y MEESEEKS_MAX_TOTAL (ver docs/diseno.md).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "caso_a.h"
#include "utils.h"
#include "common.h"

extern config_t g_config;

float caso_a_obtener_dificultad(const char *solicitud) {
    (void)solicitud;
    if (g_config.dificultad_modo == 1) {
        /* Caso 1: consultar al usuario */
        char buf[64];
        float d = -1;
        while (d < 0 || d > 100) {
            printf("Dificultad de la solicitud [0=imposible .. 100=muy fácil]: ");
            fflush(stdout);
            if (!fgets(buf, sizeof(buf), stdin)) return 50.0f;
            d = (float)atof(buf);
            if (d < 0 || d > 100) printf("Valor inválido, intente 0-100.\n");
        }
        return d;
    }
    /* Caso 2: suerte del Meeseeks, uniforme 0-100.
     * Distribución uniforme U(0,100): cada dificultad equiprobable. */
    return rand_float(0.0f, 100.0f);
}

double caso_a_tiempo_simulacion(void) {
    float lo = g_config.sim_min_seg, hi = g_config.sim_max_seg;
    if (lo < 0) lo = 0.5f;
    if (hi < lo) hi = lo;
    return (double)rand_float(lo, hi);
}

int caso_a_intentar_resolver(float dificultad) {
    if (dificultad <= 0.0f) return 0; /* imposible: jamás resuelve */
    if (dificultad >= 100.0f) return 1;
    /* Apuesta probabilística: éxito con prob. dificultad/100 */
    float tiro = rand_float(0.0f, 100.0f);
    return tiro < dificultad ? 1 : 0;
}

int caso_a_num_hijos(float dificultad) {
    if (dificultad > 85.0f) return 0;   /* 85.01-100: 1 solo lo logra */
    if (dificultad > 45.0f) return 2;   /* 45.01-85: 2 hijos N+1, i=1,2 */
    return 5;                            /* 0-45: 5 hijos N+1, i=1..5 */
}

float caso_a_diluir(float dificultad, int num_hijos) {
    if (dificultad <= 0.0f) return 0.0f; /* lo imposible no se diluye */
    if (num_hijos <= 0) return dificultad;
    /* Aging: hija = 100 - (100-padre)/(hijos+1), + jitter [-2,+2] */
    float hija = 100.0f - (100.0f - dificultad) / (float)(num_hijos + 1);
    hija += rand_float(-2.0f, 2.0f);
    if (hija > 100.0f) hija = 100.0f;
    if (hija < 0.0f) hija = 0.0f;
    return hija;
}
