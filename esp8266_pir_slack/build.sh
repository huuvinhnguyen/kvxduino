#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${FQBN:-esp8266:esp8266:nodemcuv2}"
OUTPUT_DIR="${OUTPUT_DIR:-${SCRIPT_DIR}/build}"
BUILD_PATH="${BUILD_PATH:-${TMPDIR:-/tmp}/esp8266_pir_slack-build}"

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "Error: arduino-cli is not installed or not in PATH." >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"
rm -rf "$BUILD_PATH"

arduino-cli compile \
  --fqbn "$FQBN" \
  --build-path "$BUILD_PATH" \
  --output-dir "$OUTPUT_DIR" \
  "$SCRIPT_DIR"

echo "Build completed: $OUTPUT_DIR"
