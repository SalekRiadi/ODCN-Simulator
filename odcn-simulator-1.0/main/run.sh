#!/bin/sh


for load in $(seq 0.05 0.05 0.95)
do
    ns main.tcl 30 5 8 8 10000000000 0.0000005 0.000001 1000000 $load
done

