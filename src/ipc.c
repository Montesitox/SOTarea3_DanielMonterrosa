/*
 * SO-T3-DanielMonterrosa
 * ipc.c - Pipes + shm + semáforos (esqueleto).
 */
#include <stddef.h>
#include "ipc.h"

static contadores_t *g_cont = NULL;

int ipc_init(void) {
    /* TODO: shm_open/mmap + sem_open/sem_init */
    return 0;
}

void ipc_cleanup(void) {
    /* TODO: munmap/shm_unlink, sem_close/unlink */
}

void ipc_contador_nuevo_meeseeks(int N) {
    (void)N;
    /* TODO: sem_wait, g_cont->total++, max_N, sem_post */
}

int ipc_contador_total(void) {
    return g_cont ? g_cont->total_meeseeks : 0;
}

int ipc_pipe_crear(int fds[2]) {
    (void)fds;
    /* TODO: pipe(fds) */
    return -1;
}

int ipc_pipe_enviar(int fd, const void *buf, size_t len) {
    (void)fd; (void)buf; (void)len;
    return -1;
}

int ipc_pipe_recibir(int fd, void *buf, size_t len) {
    (void)fd; (void)buf; (void)len;
    return -1;
}
