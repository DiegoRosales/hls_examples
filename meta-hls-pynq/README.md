# meta-hls-pynq

A thin Yocto layer that adds a **Jupyter Lab** server to the AMD EDF
`edf-linux-disk-image` used by this project (Digilent Zybo Z7-10,
Zynq-7000 / armv7l, scarthgap).

This is **Stage 1**: the Jupyter server only. The Jupyter packages themselves
come from the `meta-jupyter` layer that is already part of the EDF manifest;
this layer just pulls them into the image and adds an autostart service.

**Stage 2** (not yet implemented) will add the `pynq` Python library
(`python3-pynq` + its `pynqmetadata` / `pynqutils` / `grpcio` dependencies and
the `libcma` C extension) so notebooks can drive the FPGA bitstream directly.
See [STAGE2-PYNQ-PLAN.md](STAGE2-PYNQ-PLAN.md) for the detailed plan.

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
