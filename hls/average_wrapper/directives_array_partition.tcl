# Solution with an II=16, array partitioned by a factor of 2,
set_directive_top -name average_wrapper "average_wrapper"
set_directive_pipeline "average_wrapper"
set_directive_array_partition -dim 1 -factor 2 -type cyclic "average_wrapper" inputs
set_directive_interface -mode ap_memory -storage_type ram_1p "average_wrapper" inputs
