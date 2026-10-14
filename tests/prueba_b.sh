#!/bin/bash
# tests/prueba_b.sh - Caso B: 3 corridas de cálculo (1 Meeseeks c/u)
set -u
BIN="${1:-bin/box}"
echo "=== Prueba B: cálculos ==="
printf 'B\n2 + 3 * 4\nB\n(10 - 4) / 2 == 3\nB\n5 > 3 && !(0 || 0)\nchao\n' | "$BIN" -t 300
