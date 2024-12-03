# Solution with input array mapped to the AXI4-Lite interface
set_directive_top -name average_wrapper "average_wrapper"
set_directive_pipeline "average_wrapper"
set_directive_array_partition -type cyclic -dim 1 -factor 2 "average_wrapper" inputs
set_directive_interface -mode s_axilite -register=true "average_wrapper" inputs
