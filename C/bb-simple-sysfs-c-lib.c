#include <stdio.h>
#include <errno.h> // for strerror(errno)
#include <fcntl.h> // flags for 'open', eg O_WRONLY, O_SYNC, etc
#include <string.h> // strlen
#include <math.h> // fabs
#include <stdlib.h> // exit

#include "bb-simple-sysfs-c-lib.h"

// File descriptors:
int fd_pwm_duty, fd_pwm_period, fd_pwm_run; // PWM
int fd_gpio_dir_direction, fd_gpio_dir_value; // GPIO pin for motor direction
int fd_gpio_stby_direction, fd_gpio_stby_value; // GPIO pin for "standby"
int fd_eqep_period, fd_eqep_position; // EQEP

int alreadySetup = 0;

///////////////////////////////////////////////////////////////////////////
// Setup / Shutdown (Must start / end your program with this!)

void setup() {
  printf("Setting up...\n");

  if( alreadySetup ) {
    printf("You've already called setup()!\n");
    return;
  }

  //  O_RDONLY, O_WRONLY, or O_RDWR
  // To guarantee synchronous I/O, O_SYNC must be used in addition to O_DIRECT.

  int w = O_WRONLY | O_SYNC; // | O_DIRECT; // sync: block until kernel finishes the r/w
  int r = O_RDONLY | O_SYNC; // | O_DIRECT;
  int rw = O_RDWR  | O_SYNC; // | O_DIRECT;

  fd_pwm_period          = open(PWM_PATH "period", w);
  fd_pwm_duty            = open(PWM_PATH "duty",   w);
  fd_pwm_run             = open(PWM_PATH "run",    w);
  fd_gpio_dir_direction  = open(GPIO_MOTORDIR_PATH "direction", w);
  fd_gpio_dir_value      = open(GPIO_MOTORDIR_PATH "value",     w);
  fd_gpio_stby_direction = open(GPIO_STBY_PATH "direction", w);
  fd_gpio_stby_value     = open(GPIO_STBY_PATH "value",     w);
  fd_eqep_period         = open(EQEP_PATH "period",    w);
  fd_eqep_position       = open(EQEP_PATH "position", rw);

  if( fd_pwm_period == -1 || fd_pwm_duty == -1 || fd_pwm_run == -1 ) {
    printf("Couldn't find " PWM_PATH ", is it in /sys/devices/...? Prob need to \n"\
	   "$ echo am33xx_pwm > $SLOTS \n"\
	   "$ echo bone_pwm_P8_34 > $SLOTS\n"\
	   "OR, re-run build_paths.sh to re-create sysfs-paths.h,"
	   "since the PWM sysfs entry changes btwn boots :(\n");
    exit(EXIT_FAILURE);
  }
  if( fd_gpio_dir_direction == -1 || fd_gpio_dir_value == -1 ) {
    printf("Couldn't find " GPIO_MOTORDIR_PATH ", prob need to \n$ echo 70 > /sys/class/gpio/export\n");
    exit(EXIT_FAILURE);
  }
  if( fd_gpio_stby_direction == -1 || fd_gpio_stby_value == -1 ) {
    printf("Couldn't find " GPIO_STBY_PATH ", prob need to \n$ echo 73 > /sys/class/gpio/export\n");
    exit(EXIT_FAILURE);
  }
  if( fd_eqep_period == -1 || fd_eqep_position == -1 ) {
    printf("Couldn't find " EQEP_PATH ", prob need to \n$ echo bone_eqep1 > $SLOTS.\n"
	   "OR, re-run build_paths.sh to re-create sysfs-paths.h,"
	   "since the EQEP sysfs entry changes btwn boots sometimes :(\n");
    exit(EXIT_FAILURE);
  }


  alreadySetup = 1;

  // PWM set to 0%, 50kHz (20,000 ns per pwm period)
  write( fd_pwm_period, NS_PER_PWM_PERIOD_STR,5 );
  rawduty(NS_PER_PWM_PERIOD_STR,5); // set to 0% duty
  stop();

  // GPIO direction
  write( fd_gpio_dir_direction, "out", 3 );
  write( fd_gpio_stby_direction, "out", 3 );
  cw();
  stby();

  // EQEP period
  write(fd_eqep_position, "0", 1);
  write(fd_eqep_period, "100000000",9);  // ns => 0.1 sec
  printf("FYI, setting the EQEP period to 100000000 ns = 0.1sec, can change in file %s function %s line %d\n",__FILE__,__FUNCTION__,__LINE__);



  printf("Done setting up.\n");

}

void shutdown() {
  close(fd_pwm_duty);
  close(fd_pwm_period);
  close(fd_pwm_run);
  close(fd_gpio_dir_direction);
  close(fd_gpio_dir_value);
  close(fd_gpio_stby_direction);
  close(fd_gpio_stby_value);
  close(fd_eqep_period);
  close(fd_eqep_position);

  alreadySetup = 0; // if someone wants to use, they'll have to call setup() again.
}


void check_setup() { if( !alreadySetup ) { printf("Oops, need to call setup() first! Exiting.\n"); exit(EXIT_FAILURE); } }

////////////////////////////////////////////////////////
// Useful functions

// GPIO
void stby()   { check_setup(); write(fd_gpio_stby_value,"1",1); }
void unstby() { check_setup(); write(fd_gpio_stby_value,"0",1); }
void cw()     { check_setup(); write(fd_gpio_dir_value, "0",1); }
void ccw()    { check_setup(); write(fd_gpio_dir_value, "1",1); }


// PWM
void stop() 		       { check_setup(); write(fd_pwm_run,"0",1);    }
void run()  		       { check_setup(); write(fd_pwm_run,"1",1);    }
void rawduty(char* d, int len) { check_setup(); write(fd_pwm_duty, d, len); }

void duty( double d ) {
  if( d>100 ) d=100;
  if( d<0   ) d=0;
  int i = (-NS_PER_PWM_PERIOD/100)*d+NS_PER_PWM_PERIOD;
  char b[50] = {0};
  snprintf(b,45,"%d",i);
  rawduty(b,strlen(b));
}

void voltage( double v ) {
  // Sign convention: pos motor voltage => motor turns cw from rest 
  double d = 100.0 * fabs(v/MAX_VOLTAGE);
  if( v>0 ) cw();
  else      ccw();
  duty( d );
}


// EQEP
int eqep_counts() {
  check_setup(); 
  char b[50] = {0};
  int i = pread(fd_eqep_position,b,45,0);
  int enc = 999;
  sscanf(b, "%d", &enc);
  return enc;
}


double shaft_angle_deg() {
  int enc = eqep_counts();
  double ang = (double)(enc) / EQEP_PER_REV * 360.0;
  return ang;
}


