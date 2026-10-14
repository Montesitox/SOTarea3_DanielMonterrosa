/*
 * SO-T3-DanielMonterrosa
 * caso_c.h - Caso C: ejecutar programa externo y ver exit code
 * Éxito = exit 0. Incluye medición de tiempo real de ejecución.
 */
#ifndef CASO_C_H
#define CASO_C_H

/* Ejecuta cmd vía fork+exec+wait. Retorna exit code en *exit_code.
 * Retorno 0 si se pudo esperar al hijo, -1 error. */
int caso_c_ejecutar(const char *cmd, int *exit_code, double *duracion_seg);

#endif
