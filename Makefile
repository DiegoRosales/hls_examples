SHELL := bash

# Remove half-written artifacts if a recipe fails, so a stale output is never
# mistaken for an up-to-date one on the next run.
.DELETE_ON_ERROR:

################################################################################
## Source inputs (change → downstream artifact rebuilds)
################################################################################
DAT_SRCS    := python/generate_dat_files.py misc/file_example_WAV_1MG.wav

## HLS projects: each is a subdirectory of hls/ holding an hls_build.tcl that
## synthesizes and exports an IP. Add a directory name here and it is picked up
## by the generic build rule below, the aggregate 'build_hls' target, and a
## per-project 'hls-<name>' alias. Per-project C++ sources are attached to the
## rule further down (see "Per-project C++ sources").
HLS_PROJECTS := fft_wrapper dma_codec_mux_wrapper

## The IP export archive produced by an HLS project's build.
hls-export   = hls/$(1)/vitis_project/hls/impl/export.zip
HLS_EXPORTS  := $(foreach p,$(HLS_PROJECTS),$(call hls-export,$(p)))

CFG         := cfg/fft_demo.cfg.json
SCRIPT_SRCS := scripts/run.tcl scripts/vivado_init.tcl scripts/common_variables.tcl \
               $(wildcard scripts/utils/*.tcl) \
               $(wildcard scripts/pack/*.tcl) \
               $(wildcard scripts/integ/*.tcl) \
               $(wildcard scripts/build_stages/*.tcl)

PYNQ_LAYER  := $(CURDIR)/meta-hls-pynq
LAYER_SRCS  := $(shell find meta-hls-pynq -type f)

NOTEBOOK_SRCS := $(shell find notebooks -type f)

## Root password baked into the rootfs. Override on the command line, e.g.:
##   LINUX_EDF_PASSWORD=hunter2 make build-linux
## The salt is fixed so the same password yields the same hash (reproducible
## builds; the image only rebuilds when the password actually changes).
LINUX_EDF_PASSWORD ?= test123
ROOT_PW_SALT       ?= zybohls00

################################################################################
## Artifacts produced by each stage (these ARE the make targets)
################################################################################
DAT_WAV       := dat/file_example_WAV_1MG.dat
DAT_FILES     := $(DAT_WAV) dat/fft_golden_output.dat
PACKAGED_CORE := target/packaged_cores/codec_unit_pack/component.xml
XSA           := target/fft_demo_integ/fft_demo_top_wrapper.xsa
SDT_DTS       := sdt-output/system-top.dts
EDF_STAMP     := edf-build/.repo/manifest.xml
MACHINE_CONF  := edf-build/build/conf/machine/zybo-z7-10-custom.conf
LAYER_STAMP   := edf-build/.stamps/pynq-layer.stamp
BOOTBIN       := edf-build/build/tmp/deploy/images/zybo-z7-10-custom/boot.bin
WIC_IMAGE     := edf-build/build/tmp/deploy/images/amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic
COPY_STAMP    := edf-build/.stamps/copy-boot-bin.stamp
NOTEBOOK_STAGE := edf-build/.stamps/notebooks-stage
PW_INC        := meta-hls-pynq/recipes-core/images/root-password.inc
PW_MARKER     := edf-build/.stamps/root-password.value

################################################################################
## Build rules
################################################################################

## Generate the .dat stimulus/golden files from the sample WAV.
$(DAT_FILES) &: $(DAT_SRCS)
	cd python && python3 ./generate_dat_files.py

## Each HLS project's IP export is built by its own Makefile (hls/<proj>/Makefile),
## which owns that project's source list and per-stage targets. Delegate via
## recursive make (FORCE so the sub-make always gets to check freshness; it
## no-ops cheaply when nothing changed, and only re-touches export.zip on a real
## change, so downstream PACK/INTEG stay incremental).
$(HLS_EXPORTS): hls/%/vitis_project/hls/impl/export.zip: FORCE
	$(MAKE) -C hls/$*

## The FFT wrapper C-sims against the generated .dat; make sure it exists before
## the sub-make runs (order-only: the sub-make itself tracks .dat content).
$(call hls-export,fft_wrapper): | $(DAT_WAV)

## Vivado PACK: package the HLS IP as a reusable core.
$(PACKAGED_CORE): $(HLS_EXPORTS) $(CFG) $(SCRIPT_SRCS)
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

## Root password -> extrausers .inc (consumed by the image bbappend via
## 'include'). The marker records the requested password+salt and only bumps
## its mtime when they change, so hashing + the image rebuild happen only on a
## real change, not on every 'make'.
$(PW_MARKER): FORCE
	@mkdir -p $(dir $@)
	@printf '%s\n%s\n' '$(LINUX_EDF_PASSWORD)' '$(ROOT_PW_SALT)' > $@.tmp
	@if cmp -s $@.tmp $@; then rm -f $@.tmp; else mv $@.tmp $@; fi

$(PW_INC): $(PW_MARKER)
	@echo "Generating root password hash -> $@"
	hash=$$(openssl passwd -6 -salt '$(ROOT_PW_SALT)' '$(LINUX_EDF_PASSWORD)' </dev/null); \
	 test -n "$$hash" || { echo "ERROR: could not generate password hash (is openssl available?)" >&2; exit 1; }; \
	 esc=$$(printf '%s' "$$hash" | sed 's/\$$/\\$$/g'); \
	{ echo "# GENERATED by the Makefile 'root-password' stage - do not edit."; \
	  echo "# Change it with:  LINUX_EDF_PASSWORD=... make build-linux"; \
	  echo "#"; \
	  echo "# Sets the password on both 'root' and the 'amd-edf' sudo user. This"; \
	  echo "# overrides the distro's amd-edf.conf EXTRA_USERS_PARAMS, reproducing"; \
	  echo "# its amd-edf user + groups but with a real password and WITHOUT the"; \
	  echo "# 'passwd-expire' that otherwise forces a password reset on first boot."; \
	  echo "# The amd-edf sudoers entry comes from EXTRA_USERS_SUDOERS and is kept."; \
	  echo "inherit extrausers"; \
	  echo "EXTRA_USERS_PARAMS = \"useradd -p '$$esc' amd-edf; groupadd -r aie; groupadd -r wayland; usermod -a -G aie,audio,input,users,video,wayland amd-edf; usermod -p '$$esc' root;\""; } > $@

## FSBL / boot.bin for the zybo machine.
$(BOOTBIN): $(MACHINE_CONF)
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=zybo-z7-10-custom bitbake xilinx-bootbin

## Linux disk image (rebuilds when the machine config or the pynq layer change).
## boot.bin is order-only: needed for copy-boot-bin, not for the image itself.
$(WIC_IMAGE): $(MACHINE_CONF) $(LAYER_STAMP) $(PW_INC) | $(BOOTBIN)
	cd edf-build && \
	source edf-init-build-env && \
	MACHINE=amd-cortexa9thf-neon-common bitbake edf-linux-disk-image

## Copy boot.bin into the wic image (partition 1 = /efi vfat), and stage the
## contents of ./notebooks plus the bitstream/hwh (extracted from the XSA) into
## /home/root/notebooks on the rootfs (partition 3 = / ext4, per the wks). The
## notebooks dir already exists there (created by the python3-pynq recipe and
## served by Jupyter), so files are copied into it individually. All in-place
## mutations of the wic → tracked via a stamp.
##
## The hand-off is renamed to fft_demo_top_wrapper.hwh so it matches the .bit
## basename, which is what PYNQ's Overlay('fft_demo_top_wrapper.bit') expects.
$(COPY_STAMP): $(WIC_IMAGE) $(BOOTBIN) $(XSA) $(NOTEBOOK_SRCS)
	rm -rf $(NOTEBOOK_STAGE)
	mkdir -p $(NOTEBOOK_STAGE)
	cp -r notebooks/. $(NOTEBOOK_STAGE)/
	cp ./subsystems/codec_unit/fw/codec_controller.py $(NOTEBOOK_STAGE)/
	unzip -o -j $(XSA) fft_demo_top_wrapper.bit -d $(NOTEBOOK_STAGE)
	unzip -o -j $(XSA) fft_demo_top.hwh -d $(NOTEBOOK_STAGE)
	mv $(NOTEBOOK_STAGE)/fft_demo_top.hwh $(NOTEBOOK_STAGE)/fft_demo_top_wrapper.hwh
	cd edf-build && \
	source edf-init-build-env && \
	cd tmp/deploy/images && \
	wic cp ./zybo-z7-10-custom/boot.bin ./amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic:1 && \
	for f in $(CURDIR)/$(NOTEBOOK_STAGE)/*; do \
	  wic cp "$$f" ./amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic:3/home/root/notebooks/ ; \
	done
	mkdir -p $(dir $@) && touch $@

################################################################################
## Friendly phony aliases (each just points at the real artifact above)
################################################################################
.PHONY: generate_dat fft_build_hls build_hls rtl_package rtl_integ sdt-output \
        edf-build gen-machine-conf add-pynq-layer root-password build-bootbin \
        build-linux copy-boot-bin build_fft_fpga test_fft clang_format FORCE \
        $(foreach p,$(HLS_PROJECTS),hls-$(p))

generate_dat:     $(DAT_FILES)

## Build every HLS project's IP export.
build_hls:        $(HLS_EXPORTS)

## Per-project alias: 'make hls-<project>' builds just that IP export, e.g.
##   make hls-dma_codec_mux_wrapper
$(foreach p,$(HLS_PROJECTS),$(eval hls-$(p): $(call hls-export,$(p))))

## Per-stage pass-through to a project's own Makefile, e.g.
##   make hls-fft_wrapper-csim
HLS_STEPS := csim csynth cosim impl package
$(foreach p,$(HLS_PROJECTS),$(foreach s,$(HLS_STEPS),\
  $(eval .PHONY: hls-$(p)-$(s))\
  $(eval hls-$(p)-$(s): ; $$(MAKE) -C hls/$(p) $(s))))

## Back-compat alias for the FFT wrapper IP.
fft_build_hls:    $(call hls-export,fft_wrapper)

rtl_package:      $(PACKAGED_CORE)
rtl_integ:        $(XSA)
sdt-output:       $(SDT_DTS)
edf-build:        $(EDF_STAMP)
gen-machine-conf: $(MACHINE_CONF)
add-pynq-layer:   $(LAYER_STAMP)
root-password:    $(PW_INC)
build-bootbin:    $(BOOTBIN)
build-linux:      $(WIC_IMAGE)
gen-sdcard-image: $(COPY_STAMP)

## Full FPGA flow up to the XSA.
build_fft_fpga:   $(XSA)

## Format C++ files
clang_format:
	find cpp/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i

test_fft:
	cd cocotb/fft_test && \
	git clean -dfx . && \
	$(MAKE)

clean:
	rm -rf edf-build
	rm -rf sdt-output
	git clean -dfx .

%_build_hls:
	cd hls/$*_wrapper;\
	vitis-run --mode hls --tcl build_hls.tcl

%_hls_gui:
	cd hls/$*_wrapper;\
	vitis -w .

%_vivado_gui:
	vivado ./target/$*_integ/$*_integ.xpr

## Empty target used to force the password marker recipe to re-evaluate every
## run (it then only updates the marker's mtime when the value changed).
FORCE: ;
