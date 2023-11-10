
set block [lindex $argv 2]

puts "Block = $block"

cd ${block}

source ./config.tcl

set top $hls_top
set fpga xc7z010clg400-1

open_project -reset hls_project
set_top $top

foreach file $cpp_files {
    add_files $file
}

open_solution -reset solution_1

set_part $fpga

create_clock -period 10.0 -name default

source ./directives.tcl

csynth_design

export_design -format ip_catalog
export_design -flow syn

exit
