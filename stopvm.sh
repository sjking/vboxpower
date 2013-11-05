#!/bin/bash
# wait for a VM to poweroff
# modified from:
# http://superuser.com/questions/547980/bash-script-to-wait-for-virtualbox-vm-shutdown

MACHINE=$1
count=0
printf "Waiting for VM %s to poweroff\n" $MACHINE 

VBoxManage controlvm $MACHINE acpipowerbutton 2> /dev/null

until [ -n "`VBoxManage showvminfo --machinereadable $MACHINE | grep -i '^VMState="poweroff"$'`" ]; do
    count=$(( $count + 1 ))
    sleep 1
#    printf "."

    # keep trying to poweroff if VM not responding, every 10 seconds
    # might happen if machine is still powering up
    if [ 0 -eq $(( $count % 9 )) ]; then
        VBoxManage controlvm $MACHINE acpipowerbutton 2> /dev/null
    fi

done

printf "VM %s has powered off.\n" $MACHINE 2> /dev/null
