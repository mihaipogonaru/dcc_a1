#!/bin/bash

# cd into the scripts dir
cd $(dirname "$0")

# get vm ip
source vm_ips.sh

scp $1 root@${VM1_IP}:~
scp $1 root@${VM2_IP}:~
