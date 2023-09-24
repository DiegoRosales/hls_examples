# Makefile

# defaults
SIM ?= icarus
TOPLEVEL_LANG ?= verilog

VERILOG_SOURCES += $(PWD)/../../hls/fft_wrapper/hls_project/solution_1/syn/verilog/*.v
# use VHDL_SOURCES for VHDL files

# TOPLEVEL is the name of the toplevel module in your Verilog or VHDL file
TOPLEVEL = fft_wrapper

# MODULE is the basename of the Python test file
MODULE = test_my_design

ifeq ($(strip $(SIM)),verilator)
COMPILE_ARGS += -Wno-WIDTH
COMPILE_ARGS += -Wno-STMTDLY
COMPILE_ARGS += -Wno-MULTITOP
COMPILE_ARGS += -Wno-CASEINCOMPLETE
COMPILE_ARGS += -Wno-SYMRSVDWORD
COMPILE_ARGS += -Wno-ZERODLY
COMPILE_ARGS += --no-timing
# VERILATOR_SIM_DEBUG := 1
EXTRA_ARGS += --trace --trace-structs
EXTRA_ARGS += -DVM_TRACE
endif

$(shell cp -f ../../hls/fft_wrapper/hls_project/solution_1/syn/verilog/*.dat .)

# include cocotb's make rules to take care of the simulator setup
include $(shell cocotb-config --makefiles)/Makefile.sim
