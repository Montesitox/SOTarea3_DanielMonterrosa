/*
 * SO-T3-DanielMonterrosa
 * ipc.h - Pipes, memoria compartida y semáforos
 *
 * - Pipes: mensaje privado con la solicitud a cada Meeseeks.
 *   Prohibido usar variable global para mensajes.
 * - Shm + sem: contador global de niveles/instancias y nº Meeseeks.
 *   Se sugiere pthreads/semáforos del SO.
 */
#ifndef IPC_H
#define IPC_H

#include <stddef.h>

typedef struct {
    int total_meeseeks;
    int max_N;
} contadores_t;

int ipc_init(void);
void ipc_cleanup(void);
void ipc_contador_nuevo_meeseeks(int N);
int ipc_contador_total(void);

int ipc_pipe_crear(int fds[2]);
int ipc_pipe_enviar(int fd, const void *buf, size_t len);
int ipc_pipe_recibir(int fd, void *buf, size_t len);

#endif
