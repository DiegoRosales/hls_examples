overlay_name := zybo
design_name := zybo

all: bit
	@echo
	@tput setaf 2 ; echo "Built $(overlay_name) successfully!"; tput sgr0;
	@echo

bit:
	# cd ../../../../../../../ && \
	# pwd && \
	# vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "GEN_XILINX_IP" && \
	# vivado -mode batch -source scripts/run.tcl -tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK+INTEG" && \
	# cd - && \
	cp ../../../../../../../target/fft_demo_integ/fft_demo_top_wrapper.hdf ./zybo.hdf
	cp ../../../../../../../target/fft_demo_integ/fft_demo_top_wrapper.xsa ./zybo.xsa
	cp ../../../../../../../target/fft_demo_integ/fft_demo_integ.runs/impl_1/fft_demo_top_wrapper.bit ./zybo.bit


clean:
	rm -rf $(overlay_name) *.jou *.log
	rm -rf NA .Xil
