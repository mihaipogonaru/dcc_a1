#!/bin/bash

# cd into the scripts dir
cd $(dirname "$0")

# get vm ip
source vm_ips.sh

ssh root@${VM1_IP} $1
ssh root@${VM2_IP} $1
