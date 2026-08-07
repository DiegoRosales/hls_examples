# meta-hls-pynq — PYNQ implementation notes

How the PYNQ support in this layer is put together, and the non-obvious
decisions behind it. For the conceptual background on XRT / xlnk / the shim see
[XRT-AND-XLNK-EXPLAINED.md](XRT-AND-XLNK-EXPLAINED.md); for a quick-start see
[README.md](README.md).

## What gets installed

- **Jupyter Lab** (`packagegroup-python3-jupyter` from `meta-jupyter`) +
  `jupyter-startup` autostart service.
- **PYNQ runtime deps** via `packagegroup-pynq-python` (numpy, cffi, matplotlib,
  libdrm, lmsensors-libsensors, …).
- **The `pynq` library** and its dependencies, from `recipes-python/pynq/`:
  - `python3-pynq_3.1.3.bb` — built from `submodules/PYNQ` via gitsm; installs
    the prebuilt `libcma.so` and bundled notebooks.
  - `python3-pynqmetadata_0.1.9.bb`, `python3-pynqutils_0.1.2.bb` — the two PyPI
    deps that no layer provided (pure-python; `pypi` + `setuptools3`).
    `pynqutils`' `python-magic-bin` dep is dropped (Windows/macOS only; Linux
    uses `python3-magic` + system libmagic).
  - `python3-zynq-cma-device.bb` — the non-XRT device shim (see below).
- **Kernel fragment** `recipes-kernel/linux/files/pynq-pl.cfg` enabling the FPGA
  manager, UIO, xlnk/APF, CMA/DMA-CMA (128 MB), and unsetting `STRICT_DEVMEM`.
- **DDR reservation** for DMA buffers (`reserved-memory pynq-dma@1f000000`,
  16 MiB) — in `cfg/zybo-compat-overlay.dts`, so it lands in the device tree via
  the `sdt-output` flow.

## The XRT problem and the shim

PYNQ 3.1 rebuilt its device layer on **XRT** (`EmbeddedDevice(XrtDevice)` →
`pyxrt`), which isn't available on Zynq-7000/armv7 (meta-xilinx `xrt` is
aarch64-only; there is no `pyxrt`). So `import pynq` works, but `Overlay(...)`
fails with `RuntimeError: No Devices Found` — and `pynq.MMIO` also routes through
the XRT device, so the whole device layer is blocked.

`zynq_cma_device.py` (shipped by `python3-zynq-cma-device`) replaces it with a
non-XRT `Device` that uses only bare Linux primitives:

- **Registration:** sets `Device.active_device` on import instead of defining
  `_probe_` — otherwise PYNQ's throwing `EmbeddedDevice._probe_` runs first and
  aborts the probe.
- **Metadata:** parses the `.hwh` directly via `RuntimeMetadataParser`, skipping
  `get_parser()`'s `xclbinutil`/XRT step.
- **MMIO:** `mmap` of `/dev/mem` (capability `MEMORY_MAPPED`).
- **DMA buffers:** a first-fit allocator over the reserved DDR region, mapped
  **non-cacheable** through `/dev/mem` (coherent → `flush`/`invalidate` are
  no-ops). The legacy xlnk `/dev/xlnk` allocator is gone from mainline kernels,
  so `libcma` isn't used for allocation.

Validated on hardware: `Overlay(..., download=False)`, IP enumeration, MMIO
read/write, and the AXI-DMA driver all work. Runs as **root** (for `/dev/mem`).
Only the AXI-DMA + MMIO feature set is covered — genuinely XRT-only features
(xclbin CU scheduling, PL-DDR banks, Alveo kernels) are out of scope and not
relevant to a Zynq-7000 streaming design.

## Building / validating

Always pin the machine — the default `MACHINE ??= qemuarm64` builds the wrong
(aarch64) world:

```
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynqmetadata
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynqutils
MACHINE=amd-cortexa9thf-neon-common bitbake python3-pynq
```

then `make build-linux`. A kernel/device-tree change (the FPGA-manager fragment,
the DDR reservation) requires a **re-flash of the SD card** — a package-only
change can go over the air with `dnf` (see the top-level notes on the package
feed).

## Gotchas worth remembering

- **pydantic:** `pynqmetadata` pins pydantic v1, but the layers ship v2 (2.7.4).
  In practice it imports fine under v2 (only a deprecation warning). If a future
  version breaks, add a `python3-pydantic` 1.10.x recipe (last v1 series, still
  supports Python 3.12) and `PREFERRED_VERSION` it, or patch pynqmetadata to
  `from pydantic.v1 import ...`.
- **Host perl:** rpm packaging of any `.pl`-shipping recipe runs the host's
  `/usr/bin/perl` (perl.prov); it must be in the FHS `targetPkgs` (it is).
- **CMA sizing:** the reserved region (16 MiB) is separate from the kernel CMA
  pool (128 MB); size it for the design's largest buffer set.
