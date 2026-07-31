SUMMARY = "Non-XRT PYNQ device shim for Zynq-7000"
DESCRIPTION = "Installs zynq_cma_device, which registers a PYNQ Device that uses \
/dev/mem for MMIO and a reserved DDR region for DMA buffers, so Overlay / MMIO / \
allocate work without XRT (which is unavailable on Zynq-7000). See \
meta-hls-pynq/PYNQ-NOTES.md."
LICENSE = "CLOSED"

SRC_URI = "file://zynq_cma_device.py"
S = "${WORKDIR}"

inherit python3-dir

# Runtime: the pynq package (whose internals it plugs into) and numpy.
RDEPENDS:${PN} += "python3-pynq python3-numpy"

do_install() {
    install -d ${D}${PYTHON_SITEPACKAGES_DIR}
    install -m 0644 ${WORKDIR}/zynq_cma_device.py ${D}${PYTHON_SITEPACKAGES_DIR}/
}

FILES:${PN} += "${PYTHON_SITEPACKAGES_DIR}/zynq_cma_device.py"

# Depends on the DDR reservation in the device tree (reserved-memory
# pynq-dma@1f000000, added via cfg/zybo-compat-overlay.dts) for the DMA path.
