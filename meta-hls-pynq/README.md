# meta-hls-pynq

A Yocto layer that adds **Jupyter Lab** and the **PYNQ** Python library to the
AMD EDF `edf-linux-disk-image` for this project (Digilent Zybo Z7-10,
Zynq-7000 / armv7l, scarthgap), so notebooks can drive the FFT bitstream from
Python.

## What it provides

- **Jupyter Lab**, autostarted at boot on `0.0.0.0:8888` serving
  `/home/root/notebooks` (`jupyter-startup`). The Jupyter packages come from the
  `meta-jupyter` layer already in the EDF manifest; this layer installs them and
  the autostart service.
- **The `pynq` library** (`python3-pynq` 3.1.3) plus its dependencies
  (`python3-pynqmetadata`, `python3-pynqutils`, `packagegroup-pynq-python`).
- **`zynq_cma_device`** — a non-XRT device shim so `Overlay` / `MMIO` /
  `allocate` actually work on Zynq-7000 (see below).
- A **kernel fragment** (FPGA manager, UIO, CMA) and a **device-tree DDR
  reservation** for DMA buffers.

See [PYNQ-NOTES.md](PYNQ-NOTES.md) for how it's built and
[XRT-AND-XLNK-EXPLAINED.md](XRT-AND-XLNK-EXPLAINED.md) for the background on XRT,
the older xlnk mechanism, and how the shim replaces both.

## Driving the PL from a notebook

PYNQ 3.1's device layer needs **XRT**, which isn't available on Zynq-7000/armv7
(`EmbeddedDevice(XrtDevice)` → `pyxrt`; meta-xilinx `xrt` is aarch64-only), so a
bare `Overlay(...)` fails with `RuntimeError: No Devices Found`. The
`zynq_cma_device` shim registers a non-XRT PYNQ `Device` — MMIO over `/dev/mem`
and DMA buffers from a DDR region reserved in the device tree
(`pynq-dma@1f000000`). Import it before building an `Overlay`:

```python
import zynq_cma_device            # registers the non-XRT device
from pynq import Overlay, allocate
ol = Overlay("design.bit", download=False)   # PL already loaded at boot; run as root
```

Run as **root** (`/dev/mem` requires it — the Jupyter server here runs as root).
The FFT bitstream is programmed at boot from `boot.bin`, so `download=False`
attaches to the running design rather than reprogramming it.

## Security note

For convenience on a local dev board the Jupyter server starts with **no token
and no password** (matching the stock PYNQ image). Do not expose the board
directly to an untrusted network. To lock it down, set a token in
`recipes-support/jupyter/files/jupyter.service` / `jupyter.init`.

## Enabling

The project Makefile adds this layer automatically via the `add-pynq-layer`
target (a dependency of `build-linux`). To do it by hand:

    cd edf-build && source edf-init-build-env
    bitbake-layers add-layer /home/diego/proj/hls_examples/meta-hls-pynq
