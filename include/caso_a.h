/*
 * SO-T3-DanielMonterrosa
 * caso_a.h - Caso A: solicitudes textuales con dificultad 0-100
 *
 * Reglas:
 *  85.01-100  -> 1 Meeseeks resuelve solo.
 *  45.01-85   -> crea 2 hijos (N+1, i=1,2).
 *   0   -45   -> crea 5 hijos (N+1, i=1..5).
 *   0         -> imposible: plan de contingencia (ver docs/diseno.md),
 *                NO solución, límite de niveles para evitar fork-bomb.
 * Dificultad se diluye (aging) en cada nivel para tareas != 0.
 */
#ifndef CASO_A_H
#define CASO_A_H

#include "common.h"

float caso_a_obtener_dificultad(const char *solicitud);
double caso_a_tiempo_simulacion(void);
int caso_a_intentar_resolver(float dificultad);
int caso_a_num_hijos(float dificultad);
float caso_a_diluir(float dificultad, int num_hijos);

#endif
