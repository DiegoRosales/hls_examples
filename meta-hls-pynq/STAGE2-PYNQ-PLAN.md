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

1. **Kernel**: enable **CMA** and **UIO** (`CONFIG_CMA`, `CONFIG_DMA_CMA`,
   `CONFIG_UIO`, `CONFIG_UIO_PDRV_GENIRQ`). Add a `cma=` bootarg sized for the
   design.
2. **Device tree**: expose the FFT IP as a UIO node so pynq/MMIO can map it. Our
   `sdt-output` flow already generates the base DT; add pynq-style UIO/bootargs
   (ZYBO-PYNQ's `petalinux_bsp` and PYNQ's `pynq_uio_zynq.dtsi` are references).
3. **Base overlay**: `Overlay('design.bit')` wants a matching `.hwh` — our Vivado
   flow already emits both. Decide how the .bit/.hwh get onto the board.
4. Validate on hardware: `Overlay(...)`, `allocate(...)`, MMIO read/write against
   the FFT registers.

Verify: a notebook runs `from pynq import Overlay, allocate`, loads the FFT
overlay, allocates a buffer, and exchanges data with the IP.

## Open questions / risks

- Does `libcma` build cleanly under scarthgap + the gcc-14 host toolchain?
- `pynqmetadata`/`pynqutils` transitive deps (may pull more pip packages needing
  recipes).
- Whether the full `Overlay`/PL-server path works on a non-official board, or
  whether we settle for the lighter `MMIO`-only route for the FFT demo.
- Zybo CMA sizing vs the 512 MB DDR on the Z7-10.
