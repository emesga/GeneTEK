open_project BlockMatcher_HLS
set_top SeqMatcherHW

# Add files
add_files HLS_v[lindex $argv 2]/globals.h
add_files HLS_v[lindex $argv 2]/Seqmatcher.cpp

# Add Testbench
add_files -tb HLS_v[lindex $argv 2]/Main.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"

open_solution "solution1" -flow_target vivado
set_part {xczu7ev-ffvc1156-2-e}
create_clock -period [lindex $argv 3] -name default
set_clock_uncertainty 0.8
config_export -display_name {Compare sequences} -format ip_catalog -output ./IP-repo/IP-repo.zip -rtl verilog -vendor EPFL_UMA -vivado_clock [lindex $argv 3]
source "./BlockMatcher_HLS_directives.tcl"

# Do something
csim_design -O
csynth_design
#cosim_design
export_design -rtl verilog -format ip_catalog -output ./IP-repo/IP-repo.zip
quit
