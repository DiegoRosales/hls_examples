# hls_examples

## Prepare the project
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK"
vivado -mode gui   -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "INTEG"

## Open Vivado GUI
vivado target/fft_demo_integ/fft_demo_integ.xpr

## Setup docker for Pynq
docker build -t ubuntu-focal-configured .
docker run -it --rm -v $(pwd):$(pwd) -v /unraid:/unraid -w $(pwd)/submodules/PYNQ/sdbuild -e REBUILD_PYNQ_SDIST=1 -e REBUILD_PYNQ_ROOTFS=1 ubuntu-focal-configured
source /unraid/tools/Xilinx/Vivado/2022.1/settings64.sh
source /unraid/tools/PetaLinux/2022.1/settings.sh
