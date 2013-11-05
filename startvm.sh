#!/bin/bash
# wait for a VM to poweron

MACHINE=$1
count=0

VBoxManage startvm $MACHINE -type headless

until [ -n "`VBoxManage showvminfo --machinereadable $MACHINE | grep -i '^VMState="running"$'`" ]; do
    count=$(( $count + 1 ))
    sleep 1
#    printf "."

    # keep trying to poweron if VM not responding, every 10 seconds
    if [ 0 -eq $(( $count % 9 )) ]; then
        VBoxManage startvm $MACHINE -type headless
    fi

done

VBoxManage controlvm $MACHINE vrde on

#printf "VM %s has powered on\n" $MACHINE
