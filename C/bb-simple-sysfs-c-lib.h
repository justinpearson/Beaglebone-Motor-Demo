#ifndef BB_SIMPLE_SYSFS_C_LIB_H
#define BB_SIMPLE_SYSFS_C_LIB_H

// sysfs entries
// If these don't exist for you, do:

//    $ export SLOTS=$(find /sys/devices -name slots)
//    $ echo am33xx_pwm > $SLOTS
//    $ echo bone_pwm_P8_34 > $SLOTS
//    $ echo bone_eqep1 > $SLOTS
//    $ echo 70 > /sys/class/gpio/export
//    $ echo 73 > /sys/class/gpio/export

// The names may change slightly from what's on your system, do:
// find /sys/devices -name duty        # for pwm dir
// find /sys/devices -iname "*eqep*"   # for EQEP dir
#define PWM_PATH           "/sys/devices/ocp.3/pwm_test_P8_34.18/"
#define GPIO_MOTORDIR_PATH "/sys/class/gpio/gpio70/"
#define GPIO_STBY_PATH     "/sys/class/gpio/gpio73/"
#define EQEP_PATH          "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/"

#define MAX_VOLTAGE 11.7 // Volts, Dell desktop power supply
#define NS_PER_PWM_PERIOD 20000
#define NS_PER_PWM_PERIOD_STR "20000"
#define EQEP_PER_REV 1500 // I counted by hand, rough estimate


// Setup
void setup();
void shutdown();

// PWM
void stop();
void run();
void rawduty(char* d, int len);
void duty( double d );
void voltage( double v );

// GPIO
void stby();
void unstby();
void cw();
void ccw();

// EQEP
int eqep_counts();
double shaft_angle_deg();



#endif // header guard
