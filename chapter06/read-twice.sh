#!/bin/bash
set -euo pipefail

FILENAME=./testfile

function echo_with_timestamp() {
  echo "$(LANG=C date): $1"
}

rm -f "${FILENAME}"

echo_with_timestamp 'start file creation'
dd if=/dev/zero of="${FILENAME}" oflag=direct bs=1M count=1K

echo_with_timestamp 'sleep 3 seconds'
sleep 3

echo_with_timestamp 'start 1st read'
cat "${FILENAME}" > /dev/null
echo_with_timestamp 'end 1st read'

echo_with_timestamp 'sleep 3 seconds'
sleep 3

echo_with_timestamp 'start 2nd 2nd read'
cat "${FILENAME}" > /dev/null
echo_with_timestamp 'end 2nd read'

rm -f "${FILENAME}"
