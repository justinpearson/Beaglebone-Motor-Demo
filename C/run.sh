#!/bin/bash

# http://askubuntu.com/questions/15853/how-can-a-script-check-if-its-being-run-as-root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# set -x #echo on

###########################
# First we just set the $SLOTS variable and load the DTOs and GPIOs.

# $ export SLOTS=$(find /sys/devices -name slots)
# $ echo am33xx_pwm > $SLOTS
# $ echo bone_pwm_P8_34 > $SLOTS
# $ echo bone_eqep1 > $SLOTS
# $ echo 70 > /sys/class/gpio/export
# $ echo 73 > /sys/class/gpio/export



# Ensure $SLOTS is defined
# http://stackoverflow.com/questions/3601515/how-to-check-if-a-variable-is-set-in-bash
if [ -z ${SLOTS+x} ]; then 
    echo "SLOTS is unset, setting it"; 
    export SLOTS=$(find /sys/devices -name slots)
else 
    echo "SLOTS is already set to '$SLOTS', good."; 
fi


# Ensure the relevant device-tree overlays are loaded
for dto in bone_pwm_P8_34 am33xx_pwm bone_eqep1; do
    if grep --quiet $dto $SLOTS; then
	echo "$dto in $SLOTS, good"
    else
	echo "$dto not in $SLOTS, loading it..."
	echo "dmesg before load:"
	dmesg | tail 
	echo $dto > $SLOTS
	echo "dmesg after load:"
	dmesg | tail
    fi
done 



# Load the GPIOs if they're not there.

pushd /sys/class/gpio/

for n in 70 73; do
    if [ ! -d "gpio$n" ]; then
	echo "Hm, GPIO $n not loaded? I'll load it."
	echo $n > /sys/class/gpio/export
    fi
done

popd


./build_paths.sh  # Figure out where the various sysfs entries are (they can change dirs btwn reboots :( )

gcc -g -c bb-simple-sysfs-c-lib.c -lm 
gcc -g -c tests.c                 -lm 

gcc -g bb-simple-sysfs-c-lib.o tests.c main.c -o main -lm


./main

