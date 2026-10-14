#!/bin/bash
# tests/prueba_a.sh - Caso A: 3 corridas (fácil, media, difícil/imposible)
# Uso: ./bin/box -m 1 < tests/input_a.txt  (modo pregunta dificultad)
# Genera input de ejemplo con dificultades 90, 60 y 0.
set -u
BIN="${1:-bin/box}"
echo "=== Prueba A: textual ==="
printf 'A\nQuiero un cafe frances\n90\nA\nQuiero aprobar SO\n60\nA\nQuiero ser omnipotente\n0\nchao\n' | "$BIN" -m 1 -t 300 --sim-min 0.5 --sim-max 1.0
