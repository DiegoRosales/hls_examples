source /unraid/tools/Xilinx/Vivado/2023.1/settings64.sh
vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK+INTEG"