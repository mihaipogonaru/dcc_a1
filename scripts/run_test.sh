#!/bin/bash

#set -x
set -e

SCRIPTS_DIR=$(dirname "$0")
source ${SCRIPTS_DIR}/vm_ips.sh

if [[ ! -f $1 ]]; then
    echo "Usage: $0 test_file"
    exit -1
fi

# make executable
make -f Makefile

# prepare the run (get times)
./${SCRIPTS_DIR}/prepare_run.sh

# send executable to vms
scp dcc_a1 root@${VM1_IP}:~
scp dcc_a1 root@${VM1_IP}:~

# send test file to vm1
scp $1 root@${VM1_IP}:~/file
# rm file from vm2
ssh root@${VM2_IP} "rm ~/file" || :

# execute the test
time ssh root@${VM2_IP} "./dcc_a1 recv file" &
time ssh root@${VM1_IP} "./dcc_a1 send file"

# get test file from vm2
scp root@${VM2_IP}:~/file ${1}_received
