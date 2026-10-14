/*
 * SO-T3-DanielMonterrosa
 * ipc.c - Pipes + shm + semáforos POSIX.
 *
 * Contadores en memoria compartida (total_meeseeks, max_N) protegidos
 * por semáforo con nombre. Los mensajes viajan SOLO por pipes.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "ipc.h"

static contadores_t *g_cont = NULL;
static sem_t *g_sem = NULL;
static int g_shm_fd = -1;

int ipc_init(void) {
    /* Limpiar restos de corridas anteriores */
    shm_unlink(IPC_SHM_NAME);
    sem_unlink(IPC_SEM_NAME);

    g_shm_fd = shm_open(IPC_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (g_shm_fd < 0) { perror("shm_open"); return -1; }
    if (ftruncate(g_shm_fd, sizeof(contadores_t)) < 0) { perror("ftruncate"); return -1; }
    g_cont = mmap(NULL, sizeof(contadores_t), PROT_READ | PROT_WRITE,
                  MAP_SHARED, g_shm_fd, 0);
    if (g_cont == MAP_FAILED) { perror("mmap"); g_cont = NULL; return -1; }
    g_cont->total_meeseeks = 0;
    g_cont->max_N = 0;

    g_sem = sem_open(IPC_SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (g_sem == SEM_FAILED) {
        /* Si ya existía, abrirlo */
        g_sem = sem_open(IPC_SEM_NAME, O_CREAT, 0666, 1);
        if (g_sem == SEM_FAILED) { perror("sem_open"); return -1; }
    }
    return 0;
}

void ipc_cleanup(void) {
    if (g_cont && g_cont != MAP_FAILED) {
        munmap(g_cont, sizeof(contadores_t));
        g_cont = NULL;
    }
    if (g_shm_fd >= 0) { close(g_shm_fd); g_shm_fd = -1; }
    shm_unlink(IPC_SHM_NAME);
    if (g_sem && g_sem != SEM_FAILED) { sem_close(g_sem); g_sem = NULL; }
    sem_unlink(IPC_SEM_NAME);
}

void ipc_contador_nuevo_meeseeks(int N) {
    if (!g_cont || !g_sem || g_sem == SEM_FAILED) return;
    sem_wait(g_sem);
    g_cont->total_meeseeks++;
    if (N > g_cont->max_N) g_cont->max_N = N;
    sem_post(g_sem);
}

int ipc_contador_total(void) {
    int v = 0;
    if (!g_cont || !g_sem || g_sem == SEM_FAILED) return 0;
    sem_wait(g_sem);
    v = g_cont->total_meeseeks;
    sem_post(g_sem);
    return v;
}

int ipc_contador_maxN(void) {
    int v = 0;
    if (!g_cont || !g_sem || g_sem == SEM_FAILED) return 0;
    sem_wait(g_sem);
    v = g_cont->max_N;
    sem_post(g_sem);
    return v;
}

int ipc_pipe_crear(int fds[2]) {
    if (pipe(fds) < 0) { perror("pipe"); return -1; }
    return 0;
}

/* Escritura exacta */
int ipc_pipe_enviar(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    const char *p = (const char *)buf;
    while (sent < len) {
        ssize_t n = write(fd, p + sent, len - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

/* Lectura exacta (0 = EOF prematuro => error) */
int ipc_pipe_recibir(int fd, void *buf, size_t len) {
    size_t got = 0;
    char *p = (char *)buf;
    while (got < len) {
        ssize_t n = read(fd, p + got, len - got);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1; /* pipe cerrado antes de completar */
        got += (size_t)n;
    }
    return 0;
}
