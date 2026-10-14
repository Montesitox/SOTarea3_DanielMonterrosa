/*
 * SO-T3-DanielMonterrosa
 * caso_b_calculo.c - Un solo Meeseeks evalúa la expresión.
 *
 * Gramática soportada (números double, espacios libres):
 *   expr       := or
 *   or         := and ("||" and)*
 *   and        := eq ("&&" eq)*
 *   eq         := rel (("==" | "!=") rel)*
 *   rel        := add ((">=" | "<=" | ">" | "<") add)*
 *   add        := mul (("+" | "-") mul)*
 *   mul        := pow (("*" | "/" | "%") pow)*
 *   pow        := unary ("^" unary)?
 *   unary      := ("!" | "-" | "+") unary | primary
 *   primary    := número | "(" expr ")"
 * Lógicos retornan 1/0. División/mod por cero = error.
 * Documentación completa y ejemplos en docs/diseno.md.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "caso_b.h"

typedef struct { const char *p; int error; char errmsg[128]; } parser_t;

static void skip_sp(parser_t *ps) {
    while (*ps->p && isspace((unsigned char)*ps->p)) ps->p++;
}
/* Definimos de abajo hacia arriba para evitar forward hell */
static double parse_expr(parser_t *ps); /* or */

static double parse_primary(parser_t *ps) {
    skip_sp(ps);
    if (*ps->p == '(') {
        ps->p++;
        double v = parse_expr(ps);
        skip_sp(ps);
        if (*ps->p != ')') {
            ps->error = 1;
            snprintf(ps->errmsg, sizeof(ps->errmsg), "se esperaba ')'");
            return 0;
        }
        ps->p++;
        return v;
    }
    if (*ps->p == '\0') {
        ps->error = 1;
        snprintf(ps->errmsg, sizeof(ps->errmsg), "expresión vacía");
        return 0;
    }
    char *end = NULL;
    double v = strtod(ps->p, &end);
    if (end == ps->p) {
        ps->error = 1;
        snprintf(ps->errmsg, sizeof(ps->errmsg),
                 "token inválido cerca de '%.10s'", ps->p);
        return 0;
    }
    ps->p = end;
    return v;
}

static double parse_unary(parser_t *ps) {
    skip_sp(ps);
    if (*ps->p == '!') {
        ps->p++;
        double v = parse_unary(ps);
        return (v == 0.0) ? 1.0 : 0.0;
    }
    if (*ps->p == '-') { ps->p++; return -parse_unary(ps); }
    if (*ps->p == '+') { ps->p++; return parse_unary(ps); }
    return parse_primary(ps);
}

static double parse_pow(parser_t *ps) {
    double base = parse_unary(ps);
    if (ps->error) return 0;
    skip_sp(ps);
    if (*ps->p == '^') {
        ps->p++;
        double exp = parse_unary(ps);
        if (ps->error) return 0;
        base = pow(base, exp);
    }
    return base;
}

static double parse_mul(parser_t *ps) {
    double v = parse_pow(ps);
    for (;;) {
        skip_sp(ps);
        if (*ps->p == '*') {
            ps->p++;
            double r = parse_pow(ps);
            if (ps->error) return 0;
            v *= r;
        } else if (*ps->p == '/') {
            ps->p++;
            double r = parse_pow(ps);
            if (ps->error) return 0;
            if (r == 0.0) {
                ps->error = 1;
                snprintf(ps->errmsg, sizeof(ps->errmsg), "división por cero");
                return 0;
            }
            v /= r;
        } else if (*ps->p == '%') {
            ps->p++;
            double r = parse_pow(ps);
            if (ps->error) return 0;
            if (r == 0.0) {
                ps->error = 1;
                snprintf(ps->errmsg, sizeof(ps->errmsg), "módulo por cero");
                return 0;
            }
            v = fmod(v, r);
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

static double parse_add(parser_t *ps) {
    double v = parse_mul(ps);
    for (;;) {
        skip_sp(ps);
        if (*ps->p == '+') {
            /* cuidado: no confundir con "++" inválido -> lo tratamos como suma unaria */
            ps->p++;
            double r = parse_mul(ps);
            if (ps->error) return 0;
            v += r;
        } else if (*ps->p == '-') {
            ps->p++;
            double r = parse_mul(ps);
            if (ps->error) return 0;
            v -= r;
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

static double parse_rel(parser_t *ps) {
    double v = parse_add(ps);
    for (;;) {
        skip_sp(ps);
        if (strncmp(ps->p, ">=", 2) == 0) {
            ps->p += 2;
            double r = parse_add(ps);
            if (ps->error) return 0;
            v = (v >= r) ? 1.0 : 0.0;
        } else if (strncmp(ps->p, "<=", 2) == 0) {
            ps->p += 2;
            double r = parse_add(ps);
            if (ps->error) return 0;
            v = (v <= r) ? 1.0 : 0.0;
        } else if (*ps->p == '>' && ps->p[1] != '=') {
            ps->p++;
            double r = parse_add(ps);
            if (ps->error) return 0;
            v = (v > r) ? 1.0 : 0.0;
        } else if (*ps->p == '<' && ps->p[1] != '=') {
            ps->p++;
            double r = parse_add(ps);
            if (ps->error) return 0;
            v = (v < r) ? 1.0 : 0.0;
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

static double parse_eq(parser_t *ps) {
    double v = parse_rel(ps);
    for (;;) {
        skip_sp(ps);
        if (strncmp(ps->p, "==", 2) == 0) {
            ps->p += 2;
            double r = parse_rel(ps);
            if (ps->error) return 0;
            v = (v == r) ? 1.0 : 0.0;
        } else if (strncmp(ps->p, "!=", 2) == 0) {
            ps->p += 2;
            double r = parse_rel(ps);
            if (ps->error) return 0;
            v = (v != r) ? 1.0 : 0.0;
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

static double parse_and(parser_t *ps) {
    double v = parse_eq(ps);
    for (;;) {
        skip_sp(ps);
        if (strncmp(ps->p, "&&", 2) == 0) {
            ps->p += 2;
            double r = parse_eq(ps);
            if (ps->error) return 0;
            v = ((v != 0.0) && (r != 0.0)) ? 1.0 : 0.0;
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

static double parse_expr(parser_t *ps) {
    double v = parse_and(ps);
    for (;;) {
        skip_sp(ps);
        if (strncmp(ps->p, "||", 2) == 0) {
            ps->p += 2;
            double r = parse_and(ps);
            if (ps->error) return 0;
            v = ((v != 0.0) || (r != 0.0)) ? 1.0 : 0.0;
        } else break;
        if (ps->error) return 0;
    }
    return v;
}

int caso_b_evaluar(const char *expr, char *out, unsigned long out_len) {
    if (!expr || !out || out_len == 0) return -1;
    /* trim */
    while (*expr && isspace((unsigned char)*expr)) expr++;
    if (*expr == '\0') {
        snprintf(out, out_len, "Error: expresión vacía");
        return -1;
    }
    parser_t ps;
    ps.p = expr;
    ps.error = 0;
    ps.errmsg[0] = '\0';
    double v = parse_expr(&ps);
    if (!ps.error) {
        skip_sp(&ps);
        if (*ps.p != '\0') {
            ps.error = 1;
            snprintf(ps.errmsg, sizeof(ps.errmsg),
                     "carácter inesperado '%c'", *ps.p);
        }
    }
    if (ps.error) {
        snprintf(out, out_len, "Error: %s", ps.errmsg);
        return -1;
    }
    snprintf(out, out_len, "%g", v);
    return 0;
}
