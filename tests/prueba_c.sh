#!/bin/bash
# tests/prueba_c.sh - Caso C: 3 corridas programa externo (éxito = exit 0)
set -u
BIN="${1:-bin/box}"
echo "=== Prueba C: programas externos ==="
printf 'C\nls /tmp\nC\nls /ruta_que_no_existe_xyz\nC\nsleep 1\nchao\n' | "$BIN" -t 300
