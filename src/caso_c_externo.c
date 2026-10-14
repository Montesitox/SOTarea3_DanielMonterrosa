/*
 * SO-T3-DanielMonterrosa
 * caso_c_externo.c - fork + exec + wait, éxito = exit 0.
 *
 * El Meeseeks hace fork; el nieto hace execl("/bin/sh","sh","-c",cmd).
 * El Meeseeks espera con waitpid y reporta WEXITSTATUS.
 * Se mide tiempo real con CLOCK_MONOTONIC.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "caso_c.h"
#include "utils.h"

int caso_c_ejecutar(const char *cmd, int *exit_code, double *duracion_seg) {
    if (!cmd || cmd[0] == '\0') {
        if (exit_code) *exit_code = -1;
        if (duracion_seg) *duracion_seg = 0.0;
        return -1;
    }
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pid_t pid = fork();
    if (pid < 0) {
        if (exit_code) *exit_code = -1;
        if (duracion_seg) *duracion_seg = 0.0;
        return -1;
    }
    if (pid == 0) {
        /* Nieto: ejecuta programa externo (mismo pattern cap. 3 Silberschatz) */
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127); /* exec falló */
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        /* reintentar si EINTR */
        continue;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double dur = timespec_diff_seg(t0, t1);
    if (duracion_seg) *duracion_seg = dur;

    int code = -1;
    if (WIFEXITED(status)) code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) code = 128 + WTERMSIG(status);
    if (exit_code) *exit_code = code;
    return 0;
}
