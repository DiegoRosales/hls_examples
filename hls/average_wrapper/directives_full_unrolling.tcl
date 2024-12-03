# Solution with an II=1, with full loop unrolling
set_directive_top -name average_wrapper "average_wrapper"
set_directive_pipeline -II 1 "average_wrapper"
set_directive_array_partition -dim 1 -type complete "average_wrapper" inputs
