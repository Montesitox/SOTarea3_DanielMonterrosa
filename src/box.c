/*
 * SO-T3-DanielMonterrosa
 * box.c - Programa Box (main): loop de solicitudes, informe final,
 *         timeout de caos planetario.
 *
 * Flujo:
 *  1. Inicializa ambiente (ipc_init, config, alarma caos).
 *  2. Loop: lee solicitud + tipo (A/B/C) hasta despedida ("chao"/"adiós"/"bye").
 *  3. Por solicitud: clock_gettime inicio, fork Meeseeks N=1,i=1,
 *     espera resultado por pipe, clock_gettime fin (solo tiempo real).
 *  4. Al cerrar: informe (# tareas, duración c/u, éxito/fracaso,
 *     # Meeseeks por tarea) + ipc_cleanup + matar hijos pendientes.
 */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "meeseeks.h"
#include "ipc.h"
#include "utils.h"

config_t g_config = {
    .caos_timeout_seg = 300,
    .dificultad_modo = 2,
    .sim_min_seg = 0.5f,
    .sim_max_seg = 5.0f
};

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    /* TODO: parsear argv (timeout, modo dificultad, tiempos sim) */
    /* TODO: ipc_init() + signal/alarm caos planetario */
    /* TODO: loop Box + informe final */
    printf("Box Mr. Meeseeks (esqueleto). Ver include/ y docs/diseno.md\n");
    return 0;
}
