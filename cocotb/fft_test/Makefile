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

$(shell cp -f ../../hls/fft_wrapper/hls_project/solution_1/syn/verilog/*.dat .)

# include cocotb's make rules to take care of the simulator setup
include $(shell cocotb-config --makefiles)/Makefile.sim
