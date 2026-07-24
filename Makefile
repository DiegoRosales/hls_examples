
.PHONY: hls
%_build_hls:
	cd hls/$*_wrapper;\
	vitis-run --mode hls --tcl build_hls.tcl

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

generate_dat:
	cd python && python ./generate_dat_files.py
