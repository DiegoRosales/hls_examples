"""Non-XRT PYNQ device backend for Zynq-7000 (armv7).

PYNQ 3.1 rebuilt its device layer on XRT: ``EmbeddedDevice(XrtDevice)`` calls
``pyxrt.device()``, and ``pyxrt`` isn't available on Zynq-7000 (XRT is
aarch64-only). The result is ``RuntimeError: No Devices Found`` when building an
``Overlay``.

This module provides a lightweight replacement device that needs no XRT:

* **MMIO** via ``/dev/mem`` (same as PYNQ's own ``EmbeddedDevice.mmap``);
* **DMA buffers** from a DDR region reserved in the device tree
  (reserved-memory ``pynq-dma@1f000000`` in cfg/zybo-compat-overlay.dts),
  mapped non-cacheable via ``/dev/mem``. The legacy xlnk allocator
  (``/dev/xlnk``) is gone from mainline Zynq kernels, so we don't use it.

It deliberately does **not** register itself through ``_probe_`` (that would let
PYNQ's XRT-based ``EmbeddedDevice._probe_`` run first and throw, aborting the
whole probe). Instead it installs itself as ``Device.active_device`` on import,
which short-circuits the probe machinery.

Usage on the board (as root -- ``/dev/mem`` needs it)::

    import zynq_cma_device          # registers itself as the active device
    from pynq import Overlay, allocate
    ol = Overlay("design.bit", download=False)   # PL already loaded at boot

The reserved region base/size default to 0x1f000000 / 16 MiB and can be
overridden with the PYNQ_CMA_BASE / PYNQ_CMA_SIZE environment variables; they
must match the reserved-memory node in the device tree.
"""

import ctypes
import mmap
import os

import numpy as np

from pynq.buffer import PynqBuffer
from pynq.pl_server.device import Device

# --- Reserved DDR region for DMA buffers -----------------------------------
# The legacy xlnk allocator (libcma -> /dev/xlnk) is gone from mainline Zynq
# kernels, so instead we hand out buffers from a DDR region reserved in the
# device tree (reserved-memory, no-map) and mapped non-cacheable via /dev/mem.
# Non-cacheable => coherent with the AXI DMA, so no flush/invalidate is needed.
# Base/size must match the reserved-memory node (cfg/zybo-compat-overlay.dts).
CMA_BASE = int(os.environ.get("PYNQ_CMA_BASE", "0x1f000000"), 0)
CMA_SIZE = int(os.environ.get("PYNQ_CMA_SIZE", "0x1000000"), 0)   # 16 MiB
_PAGE = mmap.PAGESIZE


class _ReservedBuffer(PynqBuffer):
    """PynqBuffer backed by a slice of the reserved DDR region."""

    def freebuffer(self):
        if getattr(self, "freed", False):
            return
        self.freed = True
        owner = getattr(self, "_owner", None)
        if owner is not None:
            owner._free(self)


class _ReservedMemory:
    """First-fit allocator over a physically-contiguous reserved DDR region.

    Buffers are page-aligned (so ``mmap`` accepts the offset) and mapped
    non-cacheable through ``/dev/mem``. Freed blocks return to a free list and
    are coalesced.
    """

    def __init__(self, device, base, size):
        self.device = device
        self.base = base
        self.size = size
        self._free_blocks = [(0, size)]   # (offset, length), sorted by offset

    def allocate(self, shape, dtype, **kwargs):
        dtype = np.dtype(dtype)
        nbytes = int(np.prod(shape)) * dtype.itemsize
        need = (nbytes + _PAGE - 1) & ~(_PAGE - 1)     # page-align for mmap
        off = self._reserve(need)
        phys = self.base + off
        fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
        try:
            m = mmap.mmap(fd, need, mmap.MAP_SHARED,
                          mmap.PROT_READ | mmap.PROT_WRITE, offset=phys)
        finally:
            os.close(fd)
        backing = (ctypes.c_char * nbytes).from_buffer(m)
        buf = _ReservedBuffer(shape, dtype, buffer=backing, device=self.device,
                              device_address=phys, coherent=True)
        buf._owner = self
        buf._mmap = m
        buf._block = (off, need)
        buf.memory = self
        return buf

    def _reserve(self, need):
        for i, (off, length) in enumerate(self._free_blocks):
            if length >= need:
                if length == need:
                    self._free_blocks.pop(i)
                else:
                    self._free_blocks[i] = (off + need, length - need)
                return off
        free = sum(l for _, l in self._free_blocks)
        raise MemoryError(
            "reserved DMA region exhausted (%d bytes free of %d) -- increase "
            "PYNQ_CMA_SIZE and the reserved-memory node" % (free, self.size))

    def _free(self, buf):
        try:
            buf._mmap.close()
        except Exception:
            pass
        self._free_blocks.append(buf._block)
        self._free_blocks.sort()
        merged = []                                    # coalesce neighbours
        for off, length in self._free_blocks:
            if merged and merged[-1][0] + merged[-1][1] == off:
                merged[-1] = (merged[-1][0], merged[-1][1] + length)
            else:
                merged.append((off, length))
        self._free_blocks = merged


class ZynqCMADevice(Device):
    """Non-XRT PYNQ device for Zynq-7000: /dev/mem MMIO + libcma buffers."""

    def __init__(self, tag="zynq-cma"):
        super().__init__(tag)
        self.name = tag
        self.capabilities = {"MEMORY_MAPPED": True}
        self.default_memory = _ReservedMemory(self, CMA_BASE, CMA_SIZE)

    # --- MMIO: map /dev/mem (verbatim from PYNQ's EmbeddedDevice.mmap) ------
    def mmap(self, base_addr, length):
        if os.geteuid() != 0:
            raise EnvironmentError("Root permissions required for /dev/mem.")
        virt_base = base_addr & ~(mmap.PAGESIZE - 1)
        virt_offset = base_addr - virt_base
        fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
        mem = mmap.mmap(
            fd,
            length + virt_offset,
            mmap.MAP_SHARED,
            mmap.PROT_READ | mmap.PROT_WRITE,
            offset=virt_base,
        )
        os.close(fd)
        return np.frombuffer(mem, np.uint32, length >> 2, virt_offset)

    # --- metadata: parse the HWH directly (no XRT) -------------------------
    # PYNQ's handler.get_parser() also synthesises an xclbin via `xclbinutil`
    # (an XRT tool we don't have) to build the XRT memory topology. We don't use
    # XRT, so we call the underlying HWH parser directly and skip that step.
    def get_bitfile_metadata(self, bitfile_name, partial=False):
        from pynq.pl_server.hwh_parser import get_hwh_name
        from pynq.metadata.runtime_metadata_parser import RuntimeMetadataParser
        from pynqmetadata.frontends import Metadata

        hwh_name = get_hwh_name(bitfile_name)
        if not os.path.isfile(hwh_name):
            raise RuntimeError("HWH metadata not found next to bitstream: "
                               + hwh_name)
        parser = RuntimeMetadataParser(Metadata(input=hwh_name))
        if not partial and hasattr(parser, "refresh_hierarchy_dict"):
            parser.refresh_hierarchy_dict()
        return parser

    # --- bitstream download via the FPGA manager (no XRT) ------------------
    def download(self, bitstream, parser=None):
        if os.geteuid() != 0:
            raise EnvironmentError("Root permissions required to program the PL.")
        from pynq.pl_server.embedded_device import _preload_binfile

        if not getattr(bitstream, "binfile_name", None):
            _preload_binfile(bitstream, parser)
        with open("/sys/class/fpga_manager/fpga0/flags", "w") as fd:
            fd.write("1" if bitstream.partial else "0")
        with open("/sys/class/fpga_manager/fpga0/firmware", "w") as fd:
            fd.write(bitstream.binfile_name)
        super().post_download(bitstream, parser, self.name)

    # coherent (non-cacheable) buffers => cache ops are no-ops
    def flush(self, bo, offset, ptr, size):
        pass

    def invalidate(self, bo, offset, ptr, size):
        pass


def register():
    """Install a ZynqCMADevice as PYNQ's active device and return it."""
    dev = ZynqCMADevice()
    Device.active_device = dev
    return dev


# Activate on import so `import zynq_cma_device` is all that's needed.
device = register()
