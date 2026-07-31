# XRT, xlnk, and the `zynq_cma_device` shim — background

Reference notes explaining *why* PYNQ can't drive the PL on this board out of the
box, what the older mechanism was, and how the compatibility shim
(`recipes-python/pynq/files/zynq_cma_device.py`) replaces both. For the
implementation and build steps see [PYNQ-NOTES.md](PYNQ-NOTES.md).

---

## 1. What XRT is

**XRT (Xilinx Runtime)** is AMD/Xilinx's unified user-space runtime for talking
to FPGA accelerators. It was designed for the **datacenter** world — Alveo PCIe
cards — where:

- The FPGA is a **peripheral across a bus** (PCIe). Host CPU and FPGA do **not**
  share memory; the card has its own DRAM banks.
- You load an **`.xclbin`** — a container bundling the bitstream *plus* metadata:
  which compute kernels exist, their register maps, and the **memory topology**
  (which DDR banks, their sizes/addresses).
- To run something: open a device handle, program the xclbin, allocate **buffer
  objects (BOs)** in card memory, `sync` them across the bus (host→device
  before, device→host after), set kernel args, launch.

Implementation: a C++ library (`libxrt_core`) talking to a kernel driver —
`xocl` for PCIe Alveo, `zocl` on Zynq — via ioctls. **`pyxrt`** is a pybind11
wrapper exposing that C++ API to Python (`pyxrt.device()`, `.load_xclbin()`,
`.alloc_bo()`, `.sync()`, …).

When PYNQ was modernized under AMD, **PYNQ 3.x adopted XRT as its single device
abstraction — even on embedded Zynq.** That is the decision that breaks this
board.

### How PYNQ 3.1 uses XRT (the code path we hit)

Everything routes through a `Device`. On embedded parts:

```
EmbeddedDevice(XrtDevice)              # pynq/pl_server/embedded_device.py
  _get_handle():  pyxrt.device(0)      # -> NameError: pyxrt not defined
  allocate():     alloc_bo() in a "PS DDR" memory bank
  MMIO:           routed through the XRT device
  metadata:       parse .hwh, THEN synthesize a .xclbin via `xclbinutil`
```

Even though a Zynq-7000's PL is memory-mapped and shares DDR with the CPU (no
PCIe, no separate card memory), PYNQ 3.1 still insists on the full XRT ceremony:
an xclbin, BOs, a `pyxrt` handle.

### Why that's fatal on Zynq-7000 (armv7)

XRT and `zocl` build for **aarch64 only** (Zynq UltraScale+, Versal).
meta-xilinx's `xrt` recipe is `COMPATIBLE_MACHINE = zynqmp/versal/versal-net`,
and there is no `pyxrt`. So on armv7:

- `import pyxrt` silently fails (wrapped in try/except),
- `EmbeddedDevice._get_handle()` throws `NameError: name 'pyxrt' is not defined`,
- the device probe registers zero devices,
- `Overlay()` dies with **`RuntimeError: No Devices Found`**.

The official PYNQ-Z2 image (also Zynq-7000) only works because PYNQ ships its
*own* patched armv7 XRT via sdbuild.

### Why the ceremony is unnecessary here

XRT's apparatus bridges a gap that **doesn't exist** on Zynq-7000:

| XRT concept        | Why it exists (Alveo)              | On Zynq-7000                         |
|--------------------|------------------------------------|--------------------------------------|
| `pyxrt.device`     | manage a PCIe card                 | PL is just memory-mapped registers   |
| xclbin             | ship bitstream + topology over bus | bitstream already in `boot.bin`; topology known from `.hwh` |
| Buffer Objects + `sync` | card has its own DRAM over PCIe | CPU and PL **share** the same DDR    |
| `zocl` driver      | manage card memory/kernels         | plain `/dev/mem` + contiguous DDR    |

CPU and PL sit on the same AXI fabric and the same DDR. Poke registers directly,
hand the DMA a physical address into shared memory. No bus, no separate
allocator, no runtime needed.

---

## 2. The pre-XRT mechanism: xlnk / CMA

Before XRT, PYNQ 2.x got DMA memory through **xlnk**. Same problem ("give me
DMA-able memory and its physical address"), solved with a dedicated Xilinx
kernel driver.

### The stack

```
PYNQ 2.x  Xlnk (Python, cffi)              pynq/xlnk.py
   |
libcma / libxlnk_cma.so  (userspace C)     cma_alloc, cma_get_phy_addr, ...
   |  ioctl() + mmap()
/dev/xlnk   (kernel driver: xlnk.ko)       drivers/staging/apf/xlnk.c  (CONFIG_XILINX_APF)
   |
Linux CMA pool  (physically-contiguous DDR reserved at boot)
```

The `libcma.so` still on the board (dated 2018) is the middle layer; the driver
beneath it (`/dev/xlnk`) is what's now missing.

### The core problem

An AXI DMA engine is dumb: you write a **physical address** into its src/dst
registers and it reads/writes DDR directly, bypassing the CPU MMU. So user code
needs memory that is:

1. **physically contiguous** — the DMA walks physical addresses linearly, but
   normal `malloc` memory is scattered across physical pages by the MMU;
2. **pinned** — the kernel must never move or swap it mid-transfer.

Ordinary Linux memory is neither. xlnk filled that gap.

### How an allocation worked

Underneath was Linux's **CMA (Contiguous Memory Allocator)** — a chunk of
physically contiguous DDR carved out at boot (`CmaTotal` in `/proc/meminfo`; the
pool still exists on the board). `cma_alloc()` did roughly:

1. `libcma` calls `ioctl(/dev/xlnk, ALLOC, {size, cacheable})`.
2. The driver allocates from CMA (`dma_alloc_coherent`/CMA) → contiguous, pinned.
3. It records the kernel↔physical mapping, returns a handle.
4. `libcma` `mmap`s it into the process → a normal userspace pointer.
5. `cma_get_phy_addr(ptr)` returns the **physical address** to program into the
   DMA registers.

The `libxlnk_cma.h` API (still on the board):

```c
void *        cma_alloc(uint32_t len, uint32_t cacheable);
unsigned long cma_get_phy_addr(void *buf);
void          cma_free(void *buf);
void          cma_flush_cache(void *buf, unsigned int phys_addr, int size);
void          cma_invalidate_cache(void *buf, unsigned int phys_addr, int size);
```

### The cacheable flag (the subtle part)

`cma_alloc` takes a `cacheable` argument, and there are explicit flush/invalidate
calls — the crux of ARM DMA correctness:

- **Cacheable buffer** → fast CPU access, but cache and DDR can disagree, so you
  do manual maintenance around every transfer:
  - before **send** (MM2S): `cma_flush_cache()` — push dirty lines to DDR so the
    DMA reads fresh data;
  - after **receive** (S2MM): `cma_invalidate_cache()` — drop stale lines so the
    CPU re-reads what the DMA wrote.
  - PYNQ's `PynqBuffer.flush()`/`.invalidate()` (`sync_to_device` /
    `sync_from_device`) map straight onto these.
- **Non-cacheable (coherent) buffer** → CPU accesses DDR directly, no cache, so
  flush/invalidate are unnecessary — at the cost of slower CPU access.

xlnk pushed the coherency decision (and the work) up to the application.

### End-to-end (PYNQ 2.x)

```python
xlnk = Xlnk()
buf  = xlnk.cma_array(shape=(256,), dtype=np.int32)  # cma_alloc + mmap
buf[:] = data
buf.flush()                          # cma_flush_cache (if cacheable)
dma.sendchannel.transfer(buf)        # writes buf.physical_address into DMA regs
dma.recvchannel.transfer(out)
dma.recvchannel.wait()
out.invalidate()                     # cma_invalidate_cache
```

`buf.physical_address` came from `cma_get_phy_addr` — the only thing the DMA
driver ever needed from the buffer, and the same contract PYNQ 3.x's DMA still
uses.

### Why it's gone

`xlnk` was **out-of-tree Xilinx staging code** (`drivers/staging/apf`).
Deprecated in favor of XRT/`zocl` and dropped from mainline Zynq kernels — which
is why `/dev/xlnk` is absent on the board although `libcma.so` and the CMA pool
remain. `libcma` is a library calling a driver that no longer answers.

---

## 3. How the `zynq_cma_device` shim works

PYNQ's device layer is **pluggable**: `DeviceMeta` lets any `Device` subclass be
the active device, and the base `Device` already implements the overlay/metadata
machinery. Only the *one* embedded implementation is XRT-bound. The shim is a
~200-line drop-in `Device` using bare Linux primitives.

**1. Registration — sidestep the broken probe.** The normal path is `_probe_()`,
but PYNQ probes in priority order and `EmbeddedDevice._probe_` *throws* before
the shim could run. So instead of a probe, on import it sets:
```python
Device.active_device = ZynqCMADevice()
```
`Overlay`/`MMIO`/`allocate` all read `Device.active_device`, so this wins without
the probe machinery ever running.

**2. Metadata — parse the `.hwh` directly.** PYNQ's handler parses the `.hwh`
*and then* synthesizes an xclbin with `xclbinutil` (an XRT tool we don't have)
purely to build XRT's memory topology. The shim calls the parser directly and
skips that:
```python
RuntimeMetadataParser(Metadata(input=<hwh>))
```
This yields the IP dict, register maps and DMA info — all `Overlay` needs — with
zero XRT.

**3. MMIO — `/dev/mem`.** Replaces XRT register access: `mmap` the IP's physical
address range from `/dev/mem`, return a numpy view, declare the `MEMORY_MAPPED`
capability so PYNQ's `MMIO` class uses it. (Works because the kernel fragment
`pynq-pl.cfg` unsets `STRICT_DEVMEM`.)

**4. DMA buffers — a device-tree reserved region.** Replaces XRT BOs *and* the
missing xlnk driver. Reserve DDR in the DT (`reserved-memory pynq-dma@1f000000`,
`no-map`, in `cfg/zybo-compat-overlay.dts`) so the kernel never touches it; hand
out page-aligned slices via a small first-fit allocator, each `mmap`'d
**non-cacheable** through `/dev/mem`, wrapped in a `PynqBuffer` whose
`physical_address` points into the region. Non-cacheable is what replaces xlnk's
cache maintenance: CPU and DMA both see raw DDR, so `flush()`/`invalidate()` are
no-ops.

### xlnk vs the shim, side by side

| | xlnk (PYNQ 2.x) | shim |
|---|---|---|
| Contiguous, pinned memory | CMA pool via `xlnk.ko` | DT `reserved-memory` (`no-map`) region |
| Userspace pointer | driver-mediated `mmap` | `mmap` of `/dev/mem` at the region's phys addr |
| Physical address for DMA | `cma_get_phy_addr()` ioctl | `region_base + offset`, known statically |
| Cache coherency | cacheable + flush/invalidate ioctls | always non-cacheable → no maintenance |
| Kernel component needed | the (now missing) `xlnk` driver | none — DT reservation + `/dev/mem` |

### The essence

XRT is a heavyweight runtime for managing a **remote accelerator across a bus
with its own memory**. On Zynq-7000 the accelerator is **local and shares your
RAM**, so the shim throws away the runtime and uses the two primitives Linux
already provides — `/dev/mem` for registers, a reserved physical DDR region for
DMA buffers — while presenting the *exact same PYNQ `Device` interface* so
`Overlay`, `MMIO`, `allocate` and the DMA drivers work unmodified. It is
essentially a modern re-creation of pre-XRT "xlnk" behavior, implemented as a
plugin into PYNQ 3.1's framework rather than a fork of it.

**Caveat:** the shim covers exactly the AXI-DMA + MMIO feature set a Zynq-7000
streaming design uses. Genuinely XRT-only features (xclbin compute-unit
scheduling, PL-DDR banks, Alveo-style kernels) are not reimplemented — they are
not relevant here.
