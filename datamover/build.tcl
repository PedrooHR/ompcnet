set command [lindex $argv 0]
set device [lindex $argv 1]

set do_syn 0
set do_export 0
set do_cosim 0

switch $command {
    "syn" {
        set do_syn 1
    }
    "ip" {
        set do_syn 1
        set do_export 1
    }
    "cosim" {
        set do_syn 1
        set do_cosim 1
    }
    "all" {
        set do_syn 1
        set do_export 1
        set do_cosim 1
    }
    default {
        puts "Unrecognized command"
        exit
    }
}

open_project build

add_files datamover.cpp -cflags "-std=c++14 -I. -I../include"

if {$do_cosim} {
    add_files -tb tb_datamover.cpp -cflags "-std=c++14 -I. -I../include"
}

set_top datamover

open_solution sol1

config_export -format xo -output [pwd]/../datamover.xo

if {$do_syn} {
    set_part $device
    create_clock -period 4 -name default
    csynth_design
}

if {$do_export} {
    export_design
}

if ${do_cosim} {
    cosim_design
}

close_project

exit