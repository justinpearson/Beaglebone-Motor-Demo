#!/bin/bash

# Make some #defines that will be #included by the bb-simple-sysfs-c-lib.h.

# The goal is to make a .h file like:
# #define PWM_PATH           "/sys/devices/ocp.3/pwm_test_P8_34.18/"
# #define GPIO_MOTORDIR_PATH "/sys/class/gpio/gpio70/"
# #define GPIO_STBY_PATH     "/sys/class/gpio/gpio73/"
# #define EQEP_PATH          "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/"

# However, the path names change slightly from boot to boot; for example:
# #define PWM_PATH           "/sys/devices/ocp.3/pwm_test_P8_34.18/"
# #define PWM_PATH           "/sys/devices/ocp.3/pwm_test_P8_34.12/"


# set -x # echo on

fname="sysfs-paths.h"
echo "Finding sysfs entries, inserting into $fname ..."
echo "// THIS FILE ($fname) IS AUTOMATICALLY GENERATED BY $0" > $fname
echo "#ifndef SYSFS_PATHS_H" >> $fname
echo "#define SYSFS_PATHS_H" >> $fname
echo "#define PWM_PATH \"$(find /sys/devices -type d -name 'pwm_test_P8_34*')/\"" >> $fname
echo "#define GPIO_MOTORDIR_PATH \"/sys/class/gpio/gpio70/\"" >> $fname
echo "#define GPIO_STBY_PATH     \"/sys/class/gpio/gpio73/\"" >> $fname
echo "#define EQEP_PATH \"$(find /sys/devices -type d -name '*.eqep')/\"" >> $fname
echo "#endif // header guard" >> $fname
echo "Here's $fname :"
cat $fname


