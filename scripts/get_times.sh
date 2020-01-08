#!/bin/bash

# set -x
set -e

# cd into the scripts dir
cd $(dirname "$0")

SLEEP_PER_BIT_MS=("100" "150" "200" "400")

for sleep in ${SLEEP_PER_BIT_MS[@]}; do
    # sleep in ns
    s_ns=$(expr $sleep \* 1000 \* 1000)
    
    c1=$(taskset 0x1 ./tester "not_runner" "$s_ns" | tail -1)
    c1_third=$(expr $c1 / 3)
    
    taskset 0x1 ./tester "runner" &
    PID=$!
    
    c2=$(taskset 0x1 ./tester "not_runner" "$s_ns" | tail -1)
    kill -9 $PID
    
    c_diff=$(expr $c1 - $c2)
    #c_diff_delta=$(expr $c1 / 100)
    #c_diff_c1_third=$(expr $c1_third - $c_diff | tr -d '-')
    
    echo "C1 $c1"
    echo "C2 $c2"
    echo "C1_third $c1_third"
    echo "C_diff(C2 - c1) $c_diff"
    #echo "C_diff_delta $c_diff_delta"
    #echo "abs(C_diff - C1_third) $c_diff_c1_third"
    
    if [[ $c_diff -ge $c1_third ]]; then
        echo "FOUND"
        echo $s_ns > times.config
        
        # We set the middle ground for th_clocks_per_bit
        # a.k.a the value that separates a 1 from a 0
        # c2 + 1/2 of our current clock diff
        clocks=$(expr $c2 + $c_diff / 2)
        echo $clocks >> times.config
        
        break
    fi
done
