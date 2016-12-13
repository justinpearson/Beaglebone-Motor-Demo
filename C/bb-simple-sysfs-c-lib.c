#include <stdio.h>
#include <errno.h> // for strerror(errno)
#include <fcntl.h> // flags for 'open', eg O_WRONLY, O_SYNC, etc
#include <string.h> // strlen
#include <math.h> // fabs

#include "bb-simple-sysfs-c-lib.h"

// File descriptors:
int fd_pwm_duty, fd_pwm_period, fd_pwm_run; // PWM
int fd_gpio_dir_direction, fd_gpio_dir_value; // GPIO pin for motor direction
int fd_gpio_stby_direction, fd_gpio_stby_value; // GPIO pin for "standby"
int fd_eqep_period, fd_eqep_position; // EQEP


// Setup / Shutdown (Must start / end your program with this!)

void setup() {
  printf("Setting up...\n");

  static int alreadySetup = 0;
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
    printf("Couldn't find " PWM_PATH ", prob need to \n$ echo am33xx_pwm > $SLOTS \n$ echo bone_pwm_P8_34 > $SLOTS\n");
  }
  if( fd_gpio_dir_direction == -1 || fd_gpio_dir_value == -1 ) {
    printf("Couldn't find " GPIO_MOTORDIR_PATH ", prob need to \n$ echo 70 > /sys/class/gpio/export\n");
  }
  if( fd_gpio_stby_direction == -1 || fd_gpio_stby_value == -1 ) {
    printf("Couldn't find " GPIO_STBY_PATH ", prob need to \n$ echo 73 > /sys/class/gpio/export\n");
  }
  if( fd_eqep_period == -1 || fd_eqep_position == -1 ) {
    printf("Couldn't find " EQEP_PATH ", prob need to \n$ echo bone_eqep1 > $SLOTS.\n");
  }



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

  alreadySetup = 1;

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
}



// Useful functions

// GPIO
void stby()   { write(fd_gpio_stby_value,"1",1); }
void unstby() { write(fd_gpio_stby_value,"0",1); }
void cw()     { write(fd_gpio_dir_value, "0",1); }
void ccw()    { write(fd_gpio_dir_value, "1",1); }


// PWM
void stop() 		       { write(fd_pwm_run,"0",1);    }
void run()  		       { write(fd_pwm_run,"1",1);    }
void rawduty(char* d, int len) { write(fd_pwm_duty, d, len); }

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







/* void test_eqep_1() { */

/*   //////////////////////////////////////////////////////// */
/*   // Test EQEP */
/*   setup(); */
/*   char b; */
/*   int i; */
/*   int max = 10; */
/*   int ctr = 0; */
/*   do { */
/*     ctr++; */
/*     if( ctr > max ) { printf("Max reached!\n"); break; } */
/*     b = 99; */
/*     i = pread(fd_eqep_position,&b,1,0); */
/*   printf("read returned: %d (%s)\n",i, strerror(errno)); */
/*   printf("byte: %d\n",b); */
/*   } while( i > 0); */
/*   shutdown(); */
/* } */






/* void test_eqep_2() { */

/*   //////////////////////////////////////////////////////// */
/*   // Test eqep 2 */
  
/*   setup(); */
/*   char b[10]; */
/*   int ii = 0; */
/*   for( ii=0; ii<5; ii++ ) { */
/*     int i = pread(fd_eqep_position,b,10,0); */
/*     printf("read returned: %d (%s)\n",i, strerror(errno)); */
/*     printf("b: %s\n",b); */
/*     int j; */
/*     for( j=0; j<i; j++ ) { */
/*       printf("byte %d: %d\n",j,b[j]); */
/*     } */
/*     sleep(1); */
/*   } */
/*   shutdown(); */
/* } */



  ////////////////////////////////////////////////////////  
  // Test the "motor direction" pin
  /*
  printf("out ");
  write( fd_gpio_dir_direction, "out", 3 );
  printf("1");
  write( fd_gpio_dir_value, "1", 1 );
  sleep(1);
  printf("0");
  write( fd_gpio_dir_value, "0", 1 );
  sleep(1);
  printf("1");
  write( fd_gpio_dir_value, "1", 1 );
  sleep(1);
  printf("0");
  write( fd_gpio_dir_value, "0", 1 );
  sleep(1);
  printf("1");
  write( fd_gpio_dir_value, "1", 1 );
  sleep(1);
  printf("0");
  write( fd_gpio_dir_value, "0", 1 );
  sleep(1);
  */

  ////////////////////////////////////////////////////////
  // Test motor dir pin, with fn calls.
  /*
  printf("out ");
  write( fd_gpio_dir_direction, "out", 3 );
  printf("ccw");
  ccw();
  sleep(1);
  printf("cw");
  cw();
  sleep(1);
  printf("ccw");
  ccw();
  sleep(1);
  printf("cw");
  cw();
  sleep(1);
  printf("ccw");
  ccw();
  sleep(1);
  printf("cw");
  cw();
  sleep(1);
  */


  ////////////////////////////////////////////////////////
  // Test out stby pin
  /*
  write( fd_gpio_stby_direction, "out", 3 );
  printf("stby");
  stby();
  sleep(1);
  printf("unstby");
  unstby();
  sleep(1);
  printf("stby");
  stby();
  sleep(1);
  printf("unstby");
  unstby();
  sleep(1);
  printf("stby");
  stby();
  sleep(1);
  printf("unstby");
  unstby();
  sleep(1); 
  printf("stby");
  stby();
  */


  ////////////////////////////////////////////////////////
  // Test PWM duty cycle.
  /*
  setup();
  unstby();
  run();
  rawduty("20000",5);
  sleep(2);
  rawduty("10000",5);
  sleep(2);
  rawduty("0",1);
  sleep(2);
  stby();
  stop();
  */


  ////////////////////////////////////////////////////////
  // Run motor a little bit
  /*
  setup();

  unstby();
  run();

  pos();
  cw();
  rawduty("20000",5);
  sleep(1);

  pos();
  ccw();
  rawduty("15000",5);
  sleep(1);

  pos();
  cw();
  rawduty("10000",5);
  sleep(1);


  pos();
  ccw();
  rawduty("5000",4);
  sleep(1);


  pos();
  cw();
  rawduty("1000",4);
  sleep(1);

  pos();
  ccw();
  rawduty("0",1);
  sleep(1);

  pos();
  cw();
  rawduty("5000",4);
  sleep(1);

  pos();
  ccw();
  rawduty("10000",5);
  sleep(1);

  pos();
  cw();
  rawduty("15000",5);
  sleep(1);

  pos();
  ccw();
  rawduty("20000",5);
  sleep(1);

  stop();
  */



  ////////////////////////////////////////////////////////
  // Test duty()

  /* setup(); */

  /* unstby(); */
  /* run(); */

  /* int duties[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0}; */
  /* int n = 11; */
  /* int i=0; */
  /* for( i=0; i<n; i++ ) { */
  /*   pos(); */
  /*   if( i%2==0 ) cw(); */
  /*   else         ccw(); */
  /*   duty(duties[i]); */
  /*   sleep(1); */
  /* } */

  /* stop(); */





/* void test_duty() { */


/*   //////////////////////////////////////////////////////// */
/*   // Test duty() with doubles: change the duty cycle sinusoidally. */

/*   setup(); */
/*   unstby(); */
/*   run(); */
/*   cw(); */

/*   double dt = 0.1; // sec, time per iteration */
/*   double max_time = 5; // sec, max time of sim */
/*   int num_iters = max_time / dt; */
/*   double freq = 1; // Hz, controls how fast the duty cycle changes */

/*   int i=0; */
/*   for( i=0; i<num_iters; i++ ) { */
/*     pos(); */
/*     double d = 50.0 + 50.0 * sin(2.0 * M_PI * freq * dt * i); */
/*     duty(d); */
/*     usleep(dt*1000000.0); */
/*   } */

/*   stop(); */
/*   shutdown(); */

/* } */
  



//}
