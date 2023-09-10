# hls_examples

## Prepare the project
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK"
vivado -mode gui   -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "INTEG"

## Open Vivado GUI
vivado target/fft_demo_integ/fft_demo_integ.xpr

## Setup docker for Pynq
docker build -t ubuntu-focal-configured .
docker run -it --rm -v $(pwd):$(pwd) -v /unraid:/unraid -w $(pwd) ubuntu-focal-configured