#!/bin/sh
set -eu

pasta_programa=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if ! command -v dosbox >/dev/null 2>&1; then
  exit 1
fi
if [ "$#" -gt 1 ]; then
  echo "Uso: $0 [v]" >&2
  exit 1
fi
modo=${1:-p}
case "$modo" in
  p|v) ;;
  *) echo "Uso: $0 [v]" >&2; exit 1 ;;
esac
exec dosbox -c "mount c \"$pasta_programa\"" -c "c:" -c "rodar $modo"
