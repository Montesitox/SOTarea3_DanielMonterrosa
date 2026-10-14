/*
 * SO-T3-DanielMonterrosa
 * meeseeks.c - fork, saludo/despedida, delegación recursiva Caso A.
 */
#include <stdio.h>
#include <unistd.h>
#include "meeseeks.h"
#include "utils.h"

void meeseeks_saludar(pid_t pid, pid_t ppid, int N, int i) {
    printf("[%d/%d] Hi, I'm Mr. Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n",
           (int)pid, (int)ppid, (int)pid, (int)ppid, N, i);
}

void meeseeks_despedirse(pid_t pid, pid_t ppid, int N, int i, const char *motivo) {
    printf("[%d/%d] Meeseeks (N=%d,i=%d) se despide: %s. ¡Adiós!\n",
           (int)pid, (int)ppid, N, i, motivo);
}

pid_t meeseeks_crear_inicial(const solicitud_t *sol, int pipe_padre_fd[2]) {
    (void)sol; (void)pipe_padre_fd;
    /* TODO: pipe() + fork(); hijo -> meeseeks_ejecutar(*sol, 1, 1, fd_lectura) */
    return -1;
}

void meeseeks_ejecutar(solicitud_t sol, int N, int i, int pipe_lectura_fd) {
    (void)sol; (void)N; (void)i; (void)pipe_lectura_fd;
    /* TODO: saludar, leer solicitud por pipe, switch tipo A/B/C,
     * TODO Caso A: intentar o fork N+1 hijos + aging + esperar pipe hijo->padre,
     *       propagar éxito hacia arriba despidiéndose en cadena. */
}
