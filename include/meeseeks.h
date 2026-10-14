/*
 * SO-T3-DanielMonterrosa
 * meeseeks.h - Ciclo de vida de un Mr. Meeseeks
 */
#ifndef MEESEEKS_H
#define MEESEEKS_H

#include <sys/types.h>
#include "common.h"

/* Crea (fork) el primer Meeseeks Nivel N=1, instancia i=1 para una solicitud.
 * Retorna PID del hijo en el padre, 0 en el hijo (que no retorna: ejecuta
 * meeseeks_ejecutar y _exit), -1 en error. */
pid_t meeseeks_crear_inicial(const solicitud_t *sol, int pipe_padre_fd[2]);

/* Código que ejecuta todo proceso Meeseeks (mismo binario).
 * N, i identifican nivel/instancia. pipe_privado trae la solicitud. */
void meeseeks_ejecutar(solicitud_t sol, int N, int i, int pipe_lectura_fd);

void meeseeks_saludar(pid_t pid, pid_t ppid, int N, int i);
void meeseeks_despedirse(pid_t pid, pid_t ppid, int N, int i, const char *motivo);

#endif
