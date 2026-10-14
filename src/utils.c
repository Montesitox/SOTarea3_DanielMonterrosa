/*
 * SO-T3-DanielMonterrosa
 * utils.c - Tiempos y utilidades.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "utils.h"

double timespec_diff_seg(struct timespec a, struct timespec b) {
    double d = (double)(b.tv_sec - a.tv_sec)
             + ((double)b.tv_nsec - (double)a.tv_nsec) / 1e9;
    return d < 0 ? 0 : d;
}

float rand_float(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

int rand_int(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}

void log_msg(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    /* Todo mensaje Meeseeks indica pid y ppid (requisito tarea) */
    fprintf(stdout, "[pid=%d ppid=%d] ", (int)getpid(), (int)getppid());
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);
    va_end(ap);
}

void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
    /* trim espacios extremos */
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

/* Palabras que cierran el Box: chao, adiós/adios, bye, salir, exit, q */
int es_despedida(const char *s) {
    if (!s) return 0;
    char tmp[64];
    strncpy(tmp, s, sizeof(tmp) - 1);
    tmp[sizeof(tmp)-1] = '\0';
    for (char *p = tmp; *p; p++) *p = (char)tolower((unsigned char)*p);
    return (strcmp(tmp, "chao") == 0 || strcmp(tmp, "adios") == 0 ||
            strcmp(tmp, "adiós") == 0 || strcmp(tmp, "adiós") == 0 ||
            strcmp(tmp, "bye") == 0 || strcmp(tmp, "salir") == 0 ||
            strcmp(tmp, "exit") == 0 || strcmp(tmp, "q") == 0 ||
            strcmp(tmp, "chau") == 0);
}
