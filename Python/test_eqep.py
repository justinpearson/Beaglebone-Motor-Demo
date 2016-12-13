#!/usr/bin/python

# Import the eQEP driver
from eqep import eQEP

# Instantiate an instance of the driver
# encoder1 = eQEP("/sys/devices/ocp.2/48302000.epwmss/48302180.eqep", eQEP.MODE_ABSOLUTE)
encoder1 = eQEP("/sys/devices/ocp.3/48302000.epwmss/48302180.eqep", eQEP.MODE_ABSOLUTE)

# /sys/devices/ocp.3/48302000.epwmss/48302180.eqep

# Set the polling period of the encoder to 0.1 seconds, or 100,000,000 nanoseconds
encoder1.set_period(100000000)

# Poll the position indefinitely.  Program will provide a position at 10 Hz
while True:
    print encoder1.poll_position()

