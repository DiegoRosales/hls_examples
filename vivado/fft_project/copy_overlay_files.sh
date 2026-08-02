#!/usr/bin/env bash
# Copies the PYNQ overlay files (hwh/bit/tcl) produced by Vivado, plus the
# companion Jupyter notebook, to the board's shares so they can be picked up
# by pynq.Overlay() and Jupyter respectively. Existing files are overwritten.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

OVERLAY_DEST="smb://192.168.2.99/xilinx/pynq/overlays/fft_project"
NOTEBOOK_DEST="smb://192.168.2.99/xilinx/jupyter_notebooks/fft_project"

SRC_HWH="${SCRIPT_DIR}/fft_project.gen/sources_1/bd/fft_project_wrapper/hw_handoff/fft_project_wrapper.hwh"
SRC_BIT="${SCRIPT_DIR}/fft_project.runs/impl_1/fft_project_wrapper_wrapper.bit"
SRC_TCL="${SCRIPT_DIR}/fft_project_wrapper.tcl"
SRC_NOTEBOOK="${REPO_ROOT}/jupyter/fft_project.ipynb"

DST_HWH="${OVERLAY_DEST}/fft_project_wrapper.hwh"
DST_BIT="${OVERLAY_DEST}/fft_project_wrapper.bit"
DST_TCL="${OVERLAY_DEST}/fft_project_wrapper.tcl"
DST_NOTEBOOK="${NOTEBOOK_DEST}/fft_project.ipynb"

for f in "$SRC_HWH" "$SRC_BIT" "$SRC_TCL" "$SRC_NOTEBOOK"; do
    if [[ ! -f "$f" ]]; then
        echo "Missing source file: $f" >&2
        exit 1
    fi
done

# -p still errors if the leaf directory already exists, so ignore that case.
gio mkdir -p "$OVERLAY_DEST" 2>/dev/null || true
gio mkdir -p "$NOTEBOOK_DEST" 2>/dev/null || true

# gio copy overwrites existing destination files by default (no -i flag).
gio copy -p "$SRC_HWH" "$DST_HWH"
gio copy -p "$SRC_BIT" "$DST_BIT"
gio copy -p "$SRC_TCL" "$DST_TCL"
gio copy -p "$SRC_NOTEBOOK" "$DST_NOTEBOOK"

echo "Copied overlay files to ${OVERLAY_DEST}"
echo "Copied notebook to ${NOTEBOOK_DEST}"
