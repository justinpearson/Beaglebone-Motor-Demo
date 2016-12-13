MY_PWM_PIN = 'P8_34'
MY_PWM_DIR_PIN = 'P8_45'
MY_PWM_STBY = 'P8_44'

# Note: assumes a few DTOs are loaded:

# root@beaglebone:/home/debian/BB_24V_Motor# cat $SLOTS
#  0: 54:PF--- 
#  1: 55:PF--- 
#  2: 56:PF--- 
#  3: 57:PF--- 
#  4: ff:P-O-L Bone-LT-eMMC-2G,00A0,Texas Instrument,BB-BONE-EMMC-2G
#  5: ff:P-O-- Bone-Black-HDMI,00A0,Texas Instrument,BB-BONELT-HDMI
#  6: ff:P-O-- Bone-Black-HDMIN,00A0,Texas Instrument,BB-BONELT-HDMIN
#  7: ff:P-O-L Override Board Name,00A0,Override Manuf,BB-ADC
#  9: ff:P-O-L Override Board Name,00A0,Override Manuf,am33xx_pwm
# 10: ff:P-O-L Override Board Name,00A0,Override Manuf,bone_eqep1
# 14: ff:P-O-L Override Board Name,00A0,Override Manuf,bone_pwm_P8_34
# root@beaglebone:/home/debian/BB_24V_Motor# 


import pdb
import time

import Adafruit_BBIO.PWM as PWM
import Adafruit_BBIO.GPIO as GPIO
from eqep import eQEP


# Note: find /sys/devices/ -iname "*qep*" to find which eqep you're using
enc = eQEP("/sys/devices/ocp.3/48302000.epwmss/48302180.eqep", eQEP.MODE_ABSOLUTE)
enc.set_period(100000000) # nsec (0.1 sec)

GPIO.setup(MY_PWM_DIR_PIN, GPIO.OUT)
GPIO.setup(MY_PWM_STBY, GPIO.OUT)

def stby():
    GPIO.output(MY_PWM_STBY, GPIO.HIGH)

def go():
    GPIO.output(MY_PWM_STBY, GPIO.LOW)

def cw():
    GPIO.output(MY_PWM_DIR_PIN, GPIO.LOW)

def ccw():
    GPIO.output(MY_PWM_DIR_PIN, GPIO.HIGH)

def duty(d=0):
    if d>100: d=100
    if d<0:   d=0
    PWM.set_duty_cycle(MY_PWM_PIN,d) 

def pos():
    return enc.get_position()

PWM.start(MY_PWM_PIN,0,50000) # duty cycle 0%, freq 50kHz.
stby()
duty(0)

duties = range(0,110,10) + range(100,-10,-10)
c = True

go()

for d in duties:
    c = not(c)
    if c: 
        cw()
    else:
        ccw()
    duty(d)
    print("cw: {}, duty: {}, enc: {}".format(c, d, pos()))
    time.sleep(1)


duty(0)
stby()    

PWM.stop(MY_PWM_PIN)
GPIO.cleanup()
PWM.cleanup()
