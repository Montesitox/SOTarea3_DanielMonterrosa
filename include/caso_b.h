/*
 * SO-T3-DanielMonterrosa
 * caso_b.h - Caso B: cálculo aritmético/lógico en 1 solo Meeseeks
 */
#ifndef CASO_B_H
#define CASO_B_H

/* Evalúa expr (ej "2 + 3 * 4", "10 % 3", "5 > 3 && 1").
 * Retorna 0 ok, -1 error. Resultado como string en out. */
int caso_b_evaluar(const char *expr, char *out, unsigned long out_len);

#endif
