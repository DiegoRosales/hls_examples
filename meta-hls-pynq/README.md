# meta-hls-pynq

A thin Yocto layer that adds a **Jupyter Lab** server to the AMD EDF
`edf-linux-disk-image` used by this project (Digilent Zybo Z7-10,
Zynq-7000 / armv7l, scarthgap).

This is **Stage 1**: the Jupyter server only. The Jupyter packages themselves
come from the `meta-jupyter` layer that is already part of the EDF manifest;
this layer just pulls them into the image and adds an autostart service.

**Stage 2** adds the `pynq` Python library (`python3-pynq` +
`python3-pynqmetadata` / `python3-pynqutils` + the `libcma` allocator) and a
kernel fragment enabling the FPGA manager / CMA / UIO.

PYNQ 3.1's device layer needs **XRT**, which isn't available on Zynq-7000/armv7
(`EmbeddedDevice(XrtDevice)` → `pyxrt`; meta-xilinx `xrt` is aarch64-only), so
`Overlay(...)` would fail with `RuntimeError: No Devices Found`. To work around
this, `python3-zynq-cma-device` ships a **`zynq_cma_device`** shim that registers
a non-XRT PYNQ `Device`: MMIO over `/dev/mem` and DMA buffers from a DDR region
reserved in the device tree (`pynq-dma@1f000000`, added via
`cfg/zybo-compat-overlay.dts`). Import it before building an `Overlay`:

```python
import zynq_cma_device            # registers the non-XRT device
from pynq import Overlay
ol = Overlay("design.bit", download=False)   # PL already loaded at boot; run as root
```

**Status:** `Overlay` + MMIO validated on hardware; the DMA `allocate()` path
needs the reserved-memory node, so re-flash an image built after this change.
See [STAGE2-PYNQ-PLAN.md](STAGE2-PYNQ-PLAN.md) for the full analysis.

## What it provides

- `recipes-core/images/edf-linux-disk-image.bbappend`
  Installs `packagegroup-python3-jupyter` (JupyterLab + Notebook) and the
  `jupyter-startup` autostart service into the image.

- `recipes-support/jupyter/jupyter-startup_1.0.bb`
  Autostarts Jupyter Lab at boot, serving `/home/root/notebooks` on
  `0.0.0.0:8888`. Ships **both** a sysvinit init script and a systemd unit,
  each self-gated by `DISTRO_FEATURES`, so it works whichever init manager the
  distro uses (the EDF distro currently defaults to sysvinit on armv7).

## Security note

For convenience on a local dev board the server starts with **no token and no
password** (matching the spirit of the stock PYNQ image). Do not expose the
board directly to an untrusted network. To lock it down, override the token in
`recipes-support/jupyter/files/jupyter.service` / `jupyter.init`.

## Enabling

The project Makefile adds this layer automatically via the `add-pynq-layer`
target (a dependency of `build-linux`). To do it by hand:

    cd edf-build && source edf-init-build-env
    bitbake-layers add-layer /home/diego/proj/hls_examples/meta-hls-pynq
