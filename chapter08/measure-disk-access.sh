#!/bin/bash
set -euo pipefail

HDD_DEVFILE=/dev/xvdc
SSD_DEVFILE=/dev/xvdb

function measure() {
  devfile=$1
  acceleration=$2
  rw=$3
  access_pattern=$4
  logfile=$5

  for size in 64 128 256 512 1024 2048 4096; do
    elapsed=$((time ./io ${devfile} ${acceleration} ${rw} ${access_pattern} ${size}) 2>&1 | awk '/real/{print $2}')
    printf "%d %s\n" $size $elapsed
  done | tee ${logfile}
}

# HDD
measure ${HDD_DEVFILE} off r seq log-hdd-seq-read.txt
measure ${HDD_DEVFILE} off w seq log-hdd-seq-write.txt

measure ${HDD_DEVFILE} off r rand log-hdd-rand-read.txt
measure ${HDD_DEVFILE} off w rand log-hdd-rand-write.txt

measure ${HDD_DEVFILE} on r seq log-hdd-seq-read-acc.txt
measure ${HDD_DEVFILE} on w seq log-hdd-seq-write-acc.txt

measure ${HDD_DEVFILE} on r rand log-hdd-rand-read-acc.txt
measure ${HDD_DEVFILE} on w rand log-hdd-rand-write-acc.txt

# SSD
measure ${SSD_DEVFILE} off r seq log-ssd-seq-read.txt
measure ${SSD_DEVFILE} off w seq log-ssd-seq-write.txt

measure ${SSD_DEVFILE} off r rand log-ssd-rand-read.txt
measure ${SSD_DEVFILE} off w rand log-ssd-rand-write.txt

measure ${SSD_DEVFILE} on r seq log-ssd-seq-read-acc.txt
measure ${SSD_DEVFILE} on w seq log-ssd-seq-write-acc.txt

measure ${SSD_DEVFILE} on r rand log-ssd-rand-read-acc.txt
measure ${SSD_DEVFILE} on w rand log-ssd-rand-write-acc.txt
