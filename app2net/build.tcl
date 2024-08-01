set command [lindex $argv 0]
set device [lindex $argv 1]

set do_syn 0
set do_export 0

switch $command {
    "syn" {
        set do_syn 1
    }
    "ip" {
        set do_syn 1
        set do_export 1
    }
    "all" {
        set do_syn 1
        set do_export 1
    }
    default {
        puts "Unrecognized command"
        exit
    }
}

open_project build

add_files app2net.cpp -cflags "-std=c++17 -I. -I../include"

set_top app2net

open_solution sol1
config_export -format xo -output [pwd]/../app2net.xo

if {$do_syn} {
    set_part $device
    create_clock -period 4 -name default
    csynth_design
}

if {$do_export} {
    export_design
}

exit
