#!/bin/bash -ux

PARTS_DIR="parts"
MERGED_FILE="controller.ino"

ALL_PARTS_LIST=""

process_dir() {
  local PARTS_DIR="$1" ; shift
  echo "# PARTS_DIR='${PARTS_DIR}'"
  local PARTS_LIST="$(ls -1d ${PARTS_DIR}/* | sort)"
  for PART in ${PARTS_LIST} ; do
    echo "# PART='${PART}'"
    if test -d "${PART}" ; then
      process_dir "${PART}"
    else
      ALL_PARTS_LIST+=" ${PART}"
    fi
  done
}
process_dir "${PARTS_DIR}"

cat ${ALL_PARTS_LIST} > "${MERGED_FILE}"
