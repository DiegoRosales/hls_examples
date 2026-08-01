# Self-contained Vitis HLS build for one component.
#
# `make` (default) runs the full flow through hls_build.tcl and produces the IP
# export; that run also (re)generates vitis_project/hls_config.cfg. The per-stage
# targets reuse that generated config to run a single step, so they require the
# default build to have run first.
WORK_DIR := vitis_project
CFG      := $(WORK_DIR)/hls_config.cfg
EXPORT   := $(WORK_DIR)/hls/impl/export.zip

VITIS    := vitis-run --mode hls
# Single-stage config-flow runs execute in the project dir (--work_dir .) and
# point at the config the default build generated under $(WORK_DIR).
RUN_CFG  := $(VITIS) --work_dir ./vitis_project/ --config ./$(CFG)

# C++/data sources this component compiles (kept in sync with the add_files
# lines in hls_build.tcl). THIS is the per-project source list.
SRCS := $(wildcard ../../cpp/fft*.cpp ../../cpp/fft*.h) \
        ../../cpp/window.h ../../cpp/input_reorder_buffer.h \
        ../../dat/file_example_WAV_1MG.dat

.PHONY: all csim csynth cosim impl package clean
all: $(EXPORT)

## Full flow + IP export (also (re)generates $(CFG)).
$(EXPORT): hls_build.tcl directives.tcl $(SRCS)
	$(VITIS) --tcl hls_build.tcl

## Single-stage runs against the generated config (need the default build first).
## NOTE: csynth has no flag in `vitis-run --help`, so it is the flagless config
## action. Fix here if your vitis-run spells it differently.
csim csynth cosim impl package: $(CFG)
csim:    ; $(RUN_CFG) --csim
csynth:  ; $(RUN_CFG)
cosim:   ; $(RUN_CFG) --cosim
impl:    ; $(RUN_CFG) --impl
package: ; $(RUN_CFG) --package

$(CFG): $(EXPORT)          ## the config is a by-product of the default build

clean: ; rm -rf $(WORK_DIR)
