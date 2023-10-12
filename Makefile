
.PHONY: hls
hls:
	cd hls;\
	vitis_hls -f run_hls.tcl fft_wrapper

test_fft:
	cd cocotb/fft_test && \
	git clean -dfx . && \
	$(MAKE)

rtl_package:
	vivado \
	-mode batch \
	-source scripts/run.tcl \
	-tclargs -cfg cfg/fft_demo.cfg.json -stages "PACK"

rtl_integ:
	vivado \
	-mode batch \
	-source scripts/run.tcl \
	-tclargs -cfg cfg/fft_demo.cfg.json -stages "INTEG"

