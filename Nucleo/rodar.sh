#!/bin/sh
set -eu

pasta_programa=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if ! command -v dosbox >/dev/null 2>&1; then
  echo "DOSBox nao encontrado no PATH." >&2
  exit 1
fi
if [ "$#" -gt 1 ]; then
  echo "Uso: $0 [n|s|d|z]" >&2
  exit 1
fi
modo=${1:-n}
case "$modo" in
  n|s|d|z) ;;
  *) echo "Uso: $0 [n|s|d|z]" >&2; exit 1 ;;
esac
# Monta somente a pasta do programa; funciona de qualquer diretorio Linux.
exec dosbox -c "mount c \"$pasta_programa\"" -c "c:" -c "rodar $modo"
