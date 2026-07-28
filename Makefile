SHELL := bash

# Remove half-written artifacts if a recipe fails, so a stale output is never
# mistaken for an up-to-date one on the next run.
.DELETE_ON_ERROR:

################################################################################
## Source inputs (change → downstream artifact rebuilds)
################################################################################
DAT_SRCS    := python/generate_dat_files.py misc/file_example_WAV_1MG.wav

HLS_SRCS    := $(wildcard hls/cpp/fft*.cpp hls/cpp/fft*.h) \
               hls/fft_wrapper/build_hls.tcl \
               hls/fft_wrapper/directives.tcl \
               hls/fft_wrapper/config.tcl

CFG         := cfg/fft_demo.cfg.json
SCRIPT_SRCS := scripts/run.tcl scripts/vivado_init.tcl scripts/common_variables.tcl \
               $(wildcard scripts/utils/*.tcl) \
               $(wildcard scripts/pack/*.tcl) \
               $(wildcard scripts/integ/*.tcl) \
               $(wildcard scripts/build_stages/*.tcl)

PYNQ_LAYER  := $(CURDIR)/meta-hls-pynq
LAYER_SRCS  := $(shell find meta-hls-pynq -type f)

################################################################################
## Artifacts produced by each stage (these ARE the make targets)
################################################################################
DAT_WAV       := dat/file_example_WAV_1MG.dat
DAT_FILES     := $(DAT_WAV) dat/fft_golden_output.dat
HLS_EXPORT    := hls/fft_wrapper/vitis_project/hls/impl/export.zip
PACKAGED_CORE := target/packaged_cores/codec_unit_pack/component.xml
XSA           := target/fft_demo_integ/fft_demo_top_wrapper.xsa
SDT_DTS       := sdt-output/system-top.dts
EDF_STAMP     := edf-build/.repo/manifest.xml
MACHINE_CONF  := edf-build/build/conf/machine/zybo-z7-10-custom.conf
LAYER_STAMP   := edf-build/.stamps/pynq-layer.stamp
BOOTBIN       := edf-build/tmp/deploy/images/zybo-z7-10-custom/boot.bin
WIC_IMAGE     := edf-build/tmp/deploy/images/amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic
COPY_STAMP    := edf-build/.stamps/copy-boot-bin.stamp

################################################################################
## Build rules
################################################################################

## Generate the .dat stimulus/golden files from the sample WAV.
$(DAT_FILES) &: $(DAT_SRCS)
	cd python && python3 ./generate_dat_files.py

## Vitis HLS: C-sim, synth, cosim and IP export for the FFT wrapper.
$(HLS_EXPORT): $(HLS_SRCS) $(DAT_WAV)
	cd hls/fft_wrapper && \
	vitis-run --mode hls --tcl build_hls.tcl

## Vivado PACK: package the HLS IP as a reusable core.
$(PACKAGED_CORE): $(HLS_EXPORT) $(CFG) $(SCRIPT_SRCS)
	vivado \
	-mode batch \
	-source scripts/run.tcl \
	-tclargs -cfg $(CFG) -stages "PACK"

## Vivado INTEG: integrate the core into the block design and export the XSA.
$(XSA): $(PACKAGED_CORE) $(CFG) $(SCRIPT_SRCS)
	vivado \
	-mode batch \
	-source scripts/run.tcl \
	-tclargs -cfg $(CFG) -stages "INTEG"

## Generate .dtsi/.dts for the Yocto flow from the XSA.
$(SDT_DTS): $(XSA) cfg/zybo-compat-overlay.dts
	sdtgen -xsa $(XSA) -dir sdt-output -user_dts ./cfg/zybo-compat-overlay.dts
	sed -i 's/compatible = "xlnx,zybo";/compatible = "xlnx,zybo", "xlnx,zynq-7000";/' sdt-output/system-top.dts

## One-time bootstrap of the Yocto / EDF sources (no source prereqs: the marker
## file exists forever once synced, so this never re-runs on its own).
$(EDF_STAMP):
	mkdir -p edf-build
	cd edf-build && \
	repo init -u https://github.com/Xilinx/yocto-manifests.git -b rel-v2026.1 -m default-edf.xml && \
	repo sync

## Regenerate the machine config whenever the device tree changes.
## edf-build is an order-only prereq: a re-sync must not force this to re-run.
$(MACHINE_CONF): $(SDT_DTS) | $(EDF_STAMP)
	cd edf-build && \
	source edf-init-build-env && \
	gen-machine-conf parse-sdt --hw-description ../../sdt-output --machine-name zybo-z7-10-custom

## Register meta-hls-pynq. Depends on MACHINE_CONF as a real prereq because
## gen-machine-conf rewrites bblayers.conf and drops the layer, so it must be
## re-added after every regeneration. The guard keeps the add idempotent.
$(LAYER_STAMP): $(LAYER_SRCS) $(MACHINE_CONF)
	cd edf-build && \
	source edf-init-build-env && \
	( bitbake-layers show-layers | grep -q meta-hls-pynq || \
	  bitbake-layers add-layer $(PYNQ_LAYER) )
	mkdir -p $(dir $@) && touch $@

## FSBL / boot.bin for the zybo machine.
$(BOOTBIN): $(MACHINE_CONF)
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=zybo-z7-10-custom bitbake xilinx-bootbin

## Linux disk image (rebuilds when the machine config or the pynq layer change).
## boot.bin is order-only: needed for copy-boot-bin, not for the image itself.
$(WIC_IMAGE): $(MACHINE_CONF) $(LAYER_STAMP) | $(BOOTBIN)
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=amd-cortexa9thf-neon-common bitbake edf-linux-disk-image

## Copy boot.bin into the wic image (in-place mutation → tracked via a stamp).
$(COPY_STAMP): $(WIC_IMAGE) $(BOOTBIN)
	cd edf-build && \
	source edf-init-build-env && \
	cd tmp/deploy/images && \
	wic cp ./zybo-z7-10-custom/boot.bin ./amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic:1
	mkdir -p $(dir $@) && touch $@

################################################################################
## Friendly phony aliases (each just points at the real artifact above)
################################################################################
.PHONY: generate_dat fft_build_hls rtl_package rtl_integ sdt-output edf-build \
        gen-machine-conf add-pynq-layer build-bootbin build-linux copy-boot-bin \
        build_fft_fpga test_fft

generate_dat:     $(DAT_FILES)
fft_build_hls:    $(HLS_EXPORT)
rtl_package:      $(PACKAGED_CORE)
rtl_integ:        $(XSA)
sdt-output:       $(SDT_DTS)
edf-build:        $(EDF_STAMP)
gen-machine-conf: $(MACHINE_CONF)
add-pynq-layer:   $(LAYER_STAMP)
build-bootbin:    $(BOOTBIN)
build-linux:      $(WIC_IMAGE)
gen-sdcard-image: $(COPY_STAMP)

## Full FPGA flow up to the XSA.
build_fft_fpga:   $(XSA)

test_fft:
	cd cocotb/fft_test && \
	git clean -dfx . && \
	$(MAKE)
