SHELL := bash

.PHONY: hls
%_build_hls: generate_dat
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

## Generate .dtsi/.dts for the Yocto flow
sdt-output:
	sdtgen -xsa ./target/fft_demo_integ/fft_demo_top_wrapper.xsa -dir sdt-output \
		-user_dts cfg/zybo-compat-overlay.dts"xlnx,zynq-7000"
	sed -i 's/compatible = "xlnx,zybo";/compatible = "xlnx,zybo", "xlnx,zynq-7000";/' sdt-output/system-top.dts

## Setup the Yocto / EDF build environment
edf-build: sdt-output
	mkdir edf-build
	cd edf-build && \
	repo init -u https://github.com/Xilinx/yocto-manifests.git -b rel-v2026.1 -m default-edf.xml && \
	repo sync

gen-machine-conf: edf-build
	cd edf-build && \
	ls && \
	source edf-init-build-env && \
	gen-machine-conf parse-sdt --hw-description ../../sdt-output --machine-name zybo-z7-10-custom

build-bootbin: gen-machine-conf
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=zybo-z7-10-custom bitbake xilinx-bootbin

build-linux: build-bootbin
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=amd-cortexa9thf-neon-common bitbake edf-linux-disk-image

copy-boot-bin:
	cd edf-build && \
	source edf-init-build-env && \
	cd tmp/deploy/images && \
	wic cp ./zybo-z7-10-custom/boot.bin ./amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic:1


generate_dat:
	cd python && python ./generate_dat_files.py

build_fft_fpga: fft_build_hls
	$(MAKE) rtl_package
	$(MAKE) rtl_integ
