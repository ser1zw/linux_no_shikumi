#!/bin/bash
set -euo pipefail

FILENAME=./testfile

function echo_with_timestamp() {
  echo "$(LANG=C date): $1"
}

rm -f "${FILENAME}"

echo_with_timestamp 'start write (file creation)'
dd if=/dev/zero of="${FILENAME}" bs=1M count=1K
echo_with_timestamp 'end write'

rm -f "${FILENAME}"
