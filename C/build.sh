#!/bin/bash

gcc -c bb-simple-sysfs-c-lib.c -lm 
gcc    bb-simple-sysfs-c-lib.o test-eqep.c         -o test-eqep
gcc    bb-simple-sysfs-c-lib.o test-voltage-sine.c -o test-voltage-sine -lm
gcc    bb-simple-sysfs-c-lib.o test-pid.c          -o test-pid -lm 


