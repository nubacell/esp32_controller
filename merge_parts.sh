#!/bin/bash -ux

PARTS_DIR="parts"
MERGED_FILE="controller.ino"

PARTS_LIST="$(ls -1 ${PARTS_DIR}/* | sort)"

cat ${PARTS_LIST} > "${MERGED_FILE}"
