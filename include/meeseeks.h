/*
 * SO-T3-DanielMonterrosa
 * meeseeks.h - Ciclo de vida de un Mr. Meeseeks
 */
#ifndef MEESEEKS_H
#define MEESEEKS_H

#include <sys/types.h>
#include "common.h"

/* Crea (fork) el primer Meeseeks Nivel N=1, instancia i=1 para una solicitud.
 * fds_padre[0] = fd escritura solicitud (Box->Meeseeks),
 * fds_padre[1] = fd lectura resultado (Meeseeks->Box).
 * Retorna PID del hijo en el padre, -1 en error.
 * En el hijo NO retorna (ejecuta nodo y _exit). */
pid_t meeseeks_crear_inicial(const solicitud_t *sol, int fds_padre[2]);

/* Código que ejecuta todo proceso Meeseeks (mismo binario).
 * Si pipe_lectura_fd >= 0, la solicitud se lee del pipe (mensaje privado);
 * si no, se usa 'sol' tal cual. No retorna (termina con _exit). */
void meeseeks_ejecutar(solicitud_t sol, int N, int i, int pipe_lectura_fd);

void meeseeks_saludar(pid_t pid, pid_t ppid, int N, int i);
void meeseeks_despedirse(pid_t pid, pid_t ppid, int N, int i, const char *motivo);

#endif
