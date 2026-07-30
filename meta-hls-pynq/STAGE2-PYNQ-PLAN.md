# meta-hls-pynq — Stage 2: the `pynq` Python library

Goal: make `from pynq import Overlay, allocate` work in a notebook on the Zybo
Z7-10, so notebooks can drive the FFT bitstream from Python.

This is the follow-on to Stage 1 (Jupyter server + PYNQ runtime dependencies).
See [README.md](README.md) for Stage 1.

## Current state (done)

- Jupyter Lab autostarts on the image (`jupyter-startup`).
- PYNQ *runtime dependencies* are installed via `packagegroup-pynq-python`
  (numpy, cffi, matplotlib, libdrm, lmsensors-libsensors, …).
- The `pynq` package **itself is not installed** — hence the `ModuleNotFoundError`.

## What we're reusing

| Piece | Location | Notes |
|---|---|---|
| `pynq` package source | `submodules/PYNQ/pynq/` | Provides `Overlay`, `allocate`, `MMIO`; `__version__` 3.1.3 |
| Existing recipe | `submodules/PYNQ/sdbuild/boot/meta-pynq/recipes-filesystem/python/python3-pynq_2.3.bb` + `python-pynq.inc` | Builds pynq from the repo + the `libcma` allocator. Langdale-era — needs porting. We already reused its RDEPENDS list for the packagegroup |
| `libcma` C extension | `submodules/PYNQ/sdbuild/packages/libsds/libcma` | Built + installed by `python-pynq.inc`'s `do_install` |
| Deps already packaged | your layers | `python3-grpcio` (meta-python), `python3-nest-asyncio` (meta-jupyter), `python3-cffi`, `python3-numpy` (poky) |
| ZYBO-PYNQ | `submodules/ZYBO-PYNQ/` | Community Zybo port: `base/base.{bit,hwh,tcl}` + petalinux BSP. Targets PYNQ's **sdbuild** (PetaLinux) flow, *not* our Yocto/EDF flow — reference only, not a drop-in |

## Key facts / constraints established

- `from pynq import Overlay, allocate` runs the *entire* `pynq/__init__.py`
  (bitstream, buffer/allocate, mmio, pl_server, gpio/uio/interrupt, pmbus…),
  and instantiates the PL/`Device` **at import time**. So the import needs
  kernel + device-tree support present, not just the package on disk.
- `allocate` ultimately goes through `pynq.pl_server.Device` → a CMA allocator
  (`libcma`). Needs a CMA pool (kernel `cma=` bootarg) and `/dev/uio*`.
- `pmbus` import is why `lmsensors-libsensors` is a dependency.
- Missing dep recipes: **`python3-pynqmetadata`** and **`python3-pynqutils`**
  (Xilinx PyPI packages, pure-python).
- Version notes: pynq wants `numpy<2.0` — we ship 1.26.4 (OK). `grpcio` is only
  imported under `PYNQ_REMOTE_DEVICES`, so it may be droppable for local use.
- Escape hatch: to only poke the FFT IP's registers, `pynq.MMIO` needs far less
  than the full `Overlay`/PL-server stack — but the current import line pulls in
  everything.

## Increment 1 — packaging (get `pynq` importable) — RECIPES WRITTEN

Recipes now live in `meta-hls-pynq/recipes-python/pynq/`:

- `python3-pynqmetadata_0.1.9.bb` (pypi, setuptools3) — RDEPENDS jsonschema,
  pydantic, ipython.
- `python3-pynqutils_0.1.2.bb` (pypi, setuptools3) — RDEPENDS pynqmetadata,
  cffi, tqdm, numpy, python3-magic. `python-magic-bin` dropped (win/mac only).
- `python3-pynq_3.1.3.bb` — built from `submodules/PYNQ` via gitsm; installs the
  prebuilt `libcma.so` and the bundled notebooks; RDEPENDS pynqmetadata,
  pynqutils, nest-asyncio, packagegroup-pynq-python.

Wired into the image via `edf-linux-disk-image.bbappend`
(`IMAGE_INSTALL += python3-pynq python3-pynq-notebooks`).

Deps already in the layers: jsonschema (poky), ipython/tqdm (meta-python),
python3-magic (poky), cffi/numpy/nest-asyncio (poky/meta-jupyter). Only
pynqmetadata + pynqutils were missing.

**Validate incrementally** — always pin the machine (the default is
`qemuarm64`, which builds the wrong, aarch64 world):
```
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynqmetadata
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynqutils
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynq
```
then rebuild the image. On the board: `python3 -c "import pynq"` (may still need
Increment 2 for the PL/Device instantiation to succeed).

Note: the FHS shell must provide `/usr/bin/perl` (added to `flake.nix`) or rpm
packaging of any `.pl`-shipping recipe fails in `do_package` (perl.prov).

### The pydantic decision (main open risk)

`pynqmetadata 0.1.9` pins **pydantic==1.9.1** (pydantic **v1**); the layers ship
**pydantic 2.7.4** (v2, breaking API changes), and 1.9.1 itself predates our
target **Python 3.12**. The recipe currently RDEPENDS the available v2. If
`import pynqmetadata` fails at runtime:
1. Add a `python3-pydantic` **1.10.x** recipe (last v1 series, supports 3.12) to
   the layer and `PREFERRED_VERSION_python3-pydantic = "1.10.x"`. Nothing else in
   the image is expected to need pydantic v2, so an image-wide v1 pin is viable.
2. If some jupyter component does need v2, patch pynqmetadata to
   `from pydantic.v1 import ...` instead.

## Increment 2 — board bring-up (make it actually run)

Needs hardware + iteration.

**Done — FPGA manager / CMA / UIO kernel fragment.** `import pynq` works, but
`Overlay(...)` raised `RuntimeError: No Devices Found`: PYNQ's
`EmbeddedDevice._probe_()` only registers a device if
`/sys/class/fpga_manager/fpga0/firmware` exists, and the EDF console kernel
lacked the FPGA manager (the DT already has the `devcfg` node). Added
`recipes-kernel/linux/linux-xlnx_%.bbappend` + `files/pynq-pl.cfg` enabling
`FPGA` + `FPGA_MGR_ZYNQ_FPGA`, `UIO`(+`PDRV_GENIRQ`), `XILINX_APF`/`DMA_APF`,
`CMA`/`DMA_CMA` (128 MB), and unsetting `STRICT_DEVMEM`. Kernel change ⇒ rebuild
image + **re-flash the SD card**.

## BLOCKER (2026-07-30): PYNQ 3.1 needs XRT, unavailable on Zynq-7000

After the kernel fragment, on-board testing hit a hard wall:

- `import pynq` works; `pynqmetadata` works under pydantic v2 (only a warning) —
  so the pydantic risk was a non-issue.
- The FPGA manager now exists (`/sys/class/fpga_manager/fpga0`, "operating").
- But `Overlay(...)` still raised `RuntimeError: No Devices Found`. Root cause:
  PYNQ 3.1 rebuilt its device model on **XRT** — `EmbeddedDevice(XrtDevice)` and
  `_get_handle()` calls `pyxrt.device()`. `import pyxrt` fails silently, so
  device construction throws `NameError: name 'pyxrt' is not defined` and the
  probe registers nothing. **`pynq.MMIO` also goes through `Device.mmap()`**, so
  the whole PYNQ device layer is blocked, not just `Overlay`.
- **Stock XRT can't build for Zynq-7000/armv7**: meta-xilinx `xrt` has
  `COMPATIBLE_MACHINE` for `zynqmp`/`versal`/`versal-net` only (all aarch64), and
  there is no `pyxrt`. Official PYNQ-Z2 3.1 images only work because PYNQ's own
  sdbuild ships a *patched* XRT for armv7.

Note: the FFT bitstream is already programmed at boot (it's inside `boot.bin`,
loaded by the FSBL), so the PL is live regardless of PYNQ.

### Resolution: the `zynq_cma_device` shim (implemented, working)

Rather than downgrade PYNQ or port XRT, we plug a **non-XRT `Device`** into
PYNQ 3.1's pluggable device layer. Module: `recipes-python/pynq/files/`
`zynq_cma_device.py`, shipped by `python3-zynq-cma-device` and imported at the
top of the notebook.

How it sidesteps XRT:
- **Registration:** it does *not* define `_probe_` (that would let PYNQ's
  throwing `EmbeddedDevice._probe_` run first). Instead it sets
  `Device.active_device` on import, short-circuiting the probe.
- **Metadata:** parses the `.hwh` directly via `RuntimeMetadataParser(Metadata(
  ...))`, skipping `get_parser()`'s `xclbinutil`/XRT step.
- **MMIO:** `mmap` of `/dev/mem` (capability `MEMORY_MAPPED`).
- **DMA buffers:** a first-fit allocator over a DDR region reserved in the DT
  (`reserved-memory pynq-dma@1f000000`, 16 MiB, added via
  `cfg/zybo-compat-overlay.dts`), mapped **non-cacheable** through `/dev/mem`
  (coherent → no flush/invalidate). The legacy xlnk `/dev/xlnk` allocator is
  gone from mainline kernels, so `libcma` isn't used.

**Validated on hardware (SSH):** `Overlay(..., download=False)` builds, IPs
enumerate (`fft_wrapper_0`, `fft_dma`, …), MMIO read/write works, the `DMA`
driver binds. Run as **root** (for `/dev/mem`).

**Remaining:** the DMA `allocate()` path needs the reserved-memory node, so it
requires a **rebuild + re-flash** of an image that includes the updated DT and
`python3-zynq-cma-device`. After that, run the notebook's DMA cells.

Alternatives kept for reference: downgrade to PYNQ 2.6 (xlnk) or port XRT to
armv7 — both much larger and now unnecessary.

## Open questions / risks

- Does `libcma` build cleanly under scarthgap + the gcc-14 host toolchain?
- `pynqmetadata`/`pynqutils` transitive deps (may pull more pip packages needing
  recipes).
- Whether the full `Overlay`/PL-server path works on a non-official board, or
  whether we settle for the lighter `MMIO`-only route for the FFT demo.
- Zybo CMA sizing vs the 512 MB DDR on the Z7-10.
