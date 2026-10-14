/*
 * SO-T3-DanielMonterrosa
 * utils.h - Tiempos reales, aleatorios, logging con pid/ppid
 */
#ifndef UTILS_H
#define UTILS_H

#include <time.h>

double timespec_diff_seg(struct timespec a, struct timespec b);
float rand_float(float min, float max);
void log_msg(const char *fmt, ...);

#endif
