/*
 * SO-T3-DanielMonterrosa
 * caso_c_externo.c - fork + exec + wait, éxito = exit 0.
 */
#include <stddef.h>
#include "caso_c.h"

int caso_c_ejecutar(const char *cmd, int *exit_code, double *duracion_seg) {
    (void)cmd;
    /* TODO: clock_gettime, fork, execl /bin/sh -c cmd, waitpid,
     * WIFEXITED/WEXITSTATUS. */
    if (exit_code) *exit_code = -1;
    if (duracion_seg) *duracion_seg = 0.0;
    return -1;
}
