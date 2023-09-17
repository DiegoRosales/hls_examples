#!/usr/bin/env bash

echo "$SHELL"

source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh
source /unraid/tools/PetaLinux/2022.1/settings.sh
export SPAM=EGGS
echo "----"
echo "source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh ; source /unraid/tools/PetaLinux/2022.1/settings.sh ; ./scripts/setup_host.sh ; make BOARDS=Zybo"
echo "./scripts/setup_host.sh"
echo "make BOARDS=Zybo"
echo "----"
cd /tmp/PYNQ/sdbuild
echo "Copying .bit, .hdf, .xsa files"

cp /tmp/input/fft_demo_top_wrapper.hdf /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.hdf
cp /tmp/input/fft_demo_top_wrapper.xsa /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.xsa
cp /tmp/input/fft_demo_integ.runs/impl_1/fft_demo_top_wrapper.bit /tmp/PYNQ/boards/Zybo/petalinux_bsp/hardware_project/zybo.bit

make BOARDS=Zybo

exec "$@"