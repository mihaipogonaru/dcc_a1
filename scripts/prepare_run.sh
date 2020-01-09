#!/bin/bash

# set -x
set -e

# cd into the scripts dir
cd $(dirname "$0")

source vm_ips.sh

gcc tester.c ../phy/phy.c ../phy/cable.c -O0 -Wall -Werror -o tester
[[ -f tester ]] || exit -1

./send_to_vms.sh "tester"
./send_to_vms.sh "get_times.sh"

./execute_on_vms.sh "./get_times.sh"

# copy the values from one vm to the other
scp root@${VM1_IP}:~/times.config times.config.tmp
scp times.config.tmp root@${VM2_IP}:~/times.config
rm times.config.tmp
