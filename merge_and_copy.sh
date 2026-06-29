#!/bin/bash -u

SCRIPT_DIR="$(dirname $0)"
${SCRIPT_DIR}/merge_parts.sh \
&& \
${SCRIPT_DIR}/copy_to_build_dir.sh
