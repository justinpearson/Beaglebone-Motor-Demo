
Beaglebone demo of 24-volt DC motor & rotary encoder.

Dec 12, 2016

Summary
---------

Here is a very simple DC motor demo for the Beaglebone Black. It reads
the shaft angle with a EQEP-based rotary encoder and drives the motor
with PWM through a motor driver. The motor driver draws power through
a disused desktop power supply's 12-Volt line.

Simple software is provided as C and Python code. 

The Python code uses Adafruit_BBIO library to interface with the I/O.

The software in `bb-simple-sysfs-c-lib.c/h` provides a very thin C
interface to the Beaglebone Black's PWM, GPIO, and EQEP sysfs
entries. A lot of stuff is hard-coded for expediency, but it's easy
for newcomers to learn how to use C to interface with the Beaglebone's
sysfs entries in a minimal sort of way.

Picture of setup
-----------------

![](Pictures/bb-24v-motor-setup.jpg)

![](Pictures/bb-24v-motor-setup2.jpg)


Quick Start
--------------

```bash
    ssh debian@10.42.0.123
    sudo su
    date -s "13 Dec 2013 13:43"
    export SLOTS=$(find /sys/devices -name slots)
    echo am33xx_pwm > $SLOTS
    echo bone_pwm_P8_34 > $SLOTS
    echo bone_eqep1 > $SLOTS
    echo 70 > /sys/class/gpio/export
    echo 73 > /sys/class/gpio/export
    ./build
    ./test-pid    
```    

Quick Start C Program
----------------------

```c
    #include "bb-simple-sysfs-c-lib.h"
    void main() {
      setup();
     
      printf("Shaft angle BEFORE (deg): %lf\n", shaft_angle_deg());
      duty(50); // 50% duty cycle
      cw();     // clockwise
      unstby(); // disable 'stby' GPIO on motor driver
      run();    // set 'run' sysfs entry for PWM

      sleep(1); // let it run for a sec.

      duty(0);  // set 'duty' to 0
      stop();   // turn off 'run' 
      stby();   // set 'stby' GPIO on motor driver

      printf("Shaft angle AFTER (deg): %lf\n", shaft_angle_deg());

      shutdown();
         
    }
```



Hardware setup
----------------


The hardware consists of:

- DC motor (Globe Motors 405A336) 
- Motor driver (LMD18201T)
- Rotary encoder (US Digital)
- Beaglebone Black
- Dell desktop power supply 
- 2 10-nF capacitors for motor driver
- Two 2N3906 transistors used for 5V-to-3.3V level-shifting the EQEP sensor


The wiring schematic is shown here:

![](Pictures/bb-24v-motor-wiring.jpg)

In particular, note that:

- The motor driver has inputs for PWM, direction, and brake. 
- Pin P8_34 is the PWM.
- Pin P8_45 (GPIO) ctrls motor direction.
- Pin P8_44 (GPIO) ctrls motor brake (standby).
- The rotary encoder puts out 5V, but the BB's GPIOs require 3.3V; the transistor circuits perform level-shifting from 5V to 3.3V. 
- The rotary encoder's EQEP signal is read by the BBB's EQEP peripheral.


Sign conventions
-----------------

- ccw is positive angle
- positive motor voltage turns motor cw
- cw 1 rev => -1500 encoder ticks


Software
----------

The file `bb-simple-sysfs-c-lib.c/h` provides a very thin C interface to the Beaglebone Black's PWM, GPIO, and EQEP sysfs entries.


For expediency, I hard-coded the sysfs entries for the PWM, two GPIOs, and
EQEP in `bb-simple-sysfs-c-lib.h`:


```c
    #define PWM_PATH           "/sys/devices/ocp.3/pwm_test_P8_34.18/"
    #define GPIO_MOTORDIR_PATH "/sys/class/gpio/gpio70/"
    #define GPIO_STBY_PATH     "/sys/class/gpio/gpio73/"
    #define EQEP_PATH          "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/"
```

If these sysfs directories don't exist, execute the following lines to create them:


```bash
    $ export SLOTS=$(find /sys/devices -name slots)
    $ echo am33xx_pwm > $SLOTS
    $ echo bone_pwm_P8_34 > $SLOTS
    $ echo bone_eqep1 > $SLOTS
    $ echo 70 > /sys/class/gpio/export
    $ echo 73 > /sys/class/gpio/export
```

Notes:

- the `slots` file on my machine lives at `/sys/devices/bone_capemgr.9/slots`.

- The Exploring Beaglebone book's Fig 6-6 shows that P8_45 (that
I connected to the motor driver's "direction" pin) is GPIO 70, and
P8_44 (I connected to "brake" / standby) is GPIO 73.

- The EQEP directory may be named slightly different; find the precise one with

```bash
    $ find /sys/devices/ -iname "*qep*"
```

- The same goes for the PWM; use `find /sys/devices/ -name duty` to find it.

- Running Python's Adafruit library wipes out the sysfs entries, e.g, Adafruit_BBIO.PWM.cleanup(), so you will have to re-echo them to recreate them.



The motor driver draws power from a Dell desktop power supply's 12V
line. I hard-coded the PWM period to 50kHz. The rotary encoder seems to have a resolution of 1500 lines per revolution:

```c
      #define MAX_VOLTAGE 11.7 // Volts, Dell desktop power supply
      #define NS_PER_PWM_PERIOD 20000 // ns per PWM period
      #define NS_PER_PWM_PERIOD_STR "20000"
      #define EQEP_PER_REV 1500 // I counted by hand, rough estimate
```


C functions provided
----------------------

- PWM
    - `stop()` / `run()`: write 0 / 1 to the "run" PWM sysfs entry
    - `rawduty(char* c, int len)`: write a string to the "duty" sysfs file: "20000" is 0% duty cycle, "0" is 100% duty cycle
    - `duty( double d )`: write 0 - 100% to the "duty" sysfs file
    - `voltage( double v)`: convert voltage v into a duty cycle & GPIO direction and change them appropriately

- GPIO
    - `stby()` / `unstby()`: set P8_44 to 1 / 0
    - `cw()` / `ccw()`: set P8_45 to 1 / 0

- EQEP
    - `int eqep_counts()`: read eqep "position" file as an int
    - `double shaft_angle_deg()`: gets eqep position and converts to degrees






Details / Notes
---------------


- Note: PWM period is set in something like `/sys/devices/ocp.3/pwm_test_P8_34.18/period` with units of "ns per PWM cycle".

- Note: in sysfs, the 'duty' file is given in ns, not %. Ex: if period is set to 20000 (ns), then duty takes value between 0 (for 100% duty cycle) to 20000 (for 0% duty cycle)

- Note: polarity is switched on pwm:

   - to do 0% duty cycle, you must write same value to 

       `/sys/devices/ocp.3/pwm_test_P8_34.18/duty` 

     as you wrote to 

       `/sys/devices/ocp.3/pwm_test_P8_34.18/period`. 

   - To get 100% duty, must write 0 to duty.


- Note: instead of using the GNU C lib functions 'fread' and 'fwrite', which may do some buffering, it is more direct to use the syscalls 'read' and 'write'.

- Note: considering using 'pread' and 'pwrite' instead of 'read' and 'write'. 'p' is for 'position', and in many examples I see folks seeking the read/write head to the beginning of the file for some reason? we need to rewind the reads (any maybe writes) to the beginning of the files each time? read/write seems to work for me, except for reading the eqep, where I use pread.




Background: sysfs entries
````````````````````````````

The BBB uses a sysfs filesystem to provide a userspace interface to
the hardware. For example, set up a 50kHz PWM on pin P8_34 like this:

    echo bone_pwm_P8_34 > /sys/devices/bone_capemgr.9/slots (or wherever your slots are)

That creates the directory

    /sys/devices/ocp.3/pwm_test_P8_34.18/   (your .18 may be different)

with files like `duty`, `period`, and `run`. Now turn on the PWM:

     echo 20000 > period  # 20000 ns per PWM cycle => 50kHz
     echo 10000 > duty    # 50% duty cycle 
     echo 1 > run



Hardware setup notes
``````````````````````

cw 1 rev: eqep changes by -1450

ccw 1 rev: eqep changes by 1500

- gearbox inside?

stby low: motor turns; hi: motor stops

pwm 10%: just barely turns. stutters. sometimes stops

dir pin low: motor turns cw; high: ccw






Troubleshooting
````````````````````

The EQEP driver isn't included in the stock BBB kernel, so `echo bone_eqep1 > $SLOTS` will fail in `dmesg`; update kernel to latest with

    cd /opt/scripts/tools/
    git pull
    sudo ./update_kernel.sh
    sudo reboot

(Source: http://elinux.org/Beagleboard:BeagleBoneBlack_Debian)

Now you should have 

    # find /lib/firmware -iname "*qep*"
    /lib/firmware/bone_eqep0-00A0.dtbo
    /lib/firmware/bone_eqep1-00A0.dtbo
    /lib/firmware/bone_eqep2b-00A0.dtbo
    /lib/firmware/bone_eqep2-00A0.dtbo

My `uname -a` shows:

    Linux beaglebone 3.8.13-bone81 #1 SMP Fri Oct 14 16:04:10 UTC 2016 armv7l GNU/Linux



Make a shell variable `SLOTS` pointing to your `slots` file that organizes your DTOs:

    $ export SLOTS=$(find /sys/devices -name slots)

On my BB, `$SLOTS` is `/sys/devices/bone_capemgr.9/slots`. 

Load Device Tree Overlays:

    $ echo am33xx_pwm > $SLOTS
    $ echo bone_pwm_P8_34 > $SLOTS
    $ echo bone_eqep1 > $SLOTS
    

Have them added automatically by adding to `/boot/uboot/uEnv.txt`:

    optargs=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN capemgr.enable_partno=BB-ADC,bone_pwm_P8_34,am33xx_pwm,bone_eqep1


