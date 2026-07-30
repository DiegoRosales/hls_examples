# Enable the kernel features PYNQ needs on Zynq-7000: the FPGA manager
# (bitstream download + device discovery), UIO, the xlnk CMA allocator and CMA.
# See STAGE2-PYNQ-PLAN.md (Increment 2). Applied as a kernel config fragment,
# which linux-xlnx (kernel-yocto) merges automatically.
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://pynq-pl.cfg"
