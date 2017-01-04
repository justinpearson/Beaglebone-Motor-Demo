#include <stdio.h>
#include <errno.h> // for strerror(errno)
// #include <fcntl.h> // flags for 'open', eg O_WRONLY, O_SYNC, etc
// #include <string.h> // strlen
#include <math.h> // fabs
//#include <stdlib.h> // exit
#include <unistd.h> // usleep

#include "bb-simple-sysfs-c-lib.h"



///////////////////////////////////////////////////////////
// Tests

void test_eqep() {

  printf("Running test: %s\n",__FUNCTION__);

  // Test shaft_angle_deg
  setup();
 
  int i=0; 
  int n=10;
  for(i=0; i<n; i++) {
    printf("%d/%d: Shaft angle (deg): %lf\n", i,n,shaft_angle_deg());
    sleep(1);
  }

  shutdown();

  printf("Test complete: %s\n",__FUNCTION__);

}


void test_voltage_sine() {

  printf("Running test: %s\n",__FUNCTION__);

  setup();
  unstby();
  run();
  cw();

  double dt = 0.1; // sec, time per iteration
  double max_time = 5; // sec, max time of sim
  int num_iters = max_time / dt;
  double freq = 1; // Hz, controls how fast the duty cycle changes

  int i=0;
  for( i=0; i<num_iters; i++ ) {
    printf("%d/%d: Shaft angle (deg): %lf\n", i,num_iters,shaft_angle_deg());
    double v = MAX_VOLTAGE * sin(2.0 * M_PI * freq * dt * i);
    voltage(v);
    double usleepfor = dt*1000000.0;
    //    printf("Sleeping for %lf us...\n",usleepfor);
    usleep(usleepfor);
  }

  voltage(0);
  stop();
  stby();
  shutdown();

  printf("Test complete: %s\n",__FUNCTION__);
}


void test_pid ( double kp ) {

  printf("Running test: %s\n",__FUNCTION__);

  setup();
  unstby();
  run();
  cw();

  //   double kp = -.015;
  //  if( argc > 1 ) {
  //    printf("input: %s\n",argv[1]);
  //    kp = atof(argv[1]);
  //  }

  printf("kp: %lf\n",kp);

  double dt = 0.1; // sec, time per iteration
  double max_time = 5; // sec, max time of sim
  int num_iters = max_time / dt;
  double freq = 1; // Hz, controls how fast the reference angle changes

  int i=0;
  for( i=0; i<num_iters; i++ ) {
    double angle = shaft_angle_deg();
    double ref = 180 * sin(2.0 * M_PI * freq * dt * i); // deg
    double error = ref-angle;
    double v = kp * error;
    printf("iter: %d, angle: %f, ref: %f, err: %f, volt: %f\n",
	   i,angle,ref,error,v);
    if( v>MAX_VOLTAGE ) v=MAX_VOLTAGE;
    if( v<-MAX_VOLTAGE) v=-MAX_VOLTAGE;
    voltage(v);
    usleep(dt*1000000.0);
  }

  stop();

  shutdown();

  printf("Test complete: %s\n",__FUNCTION__);

}





///////////////////////////////////////////
// Preliminary tests (they don't necessarily use the lib, they're for debugging)

void test_eqep_1() {

  extern int fd_eqep_position;

  printf("Running test: %s\n",__FUNCTION__);
  setup();
  char b;
  int i;
  int max = 10;
  int ctr = 0;
  do {
    ctr++;
    if( ctr > max ) { printf("Max reached!\n"); break; }
    b = 99;
    i = pread(fd_eqep_position,&b,1,0);
  printf("read returned: %d (%s)\n",i, strerror(errno));
  printf("byte: %d\n",b);
  } while( i > 0);
  shutdown();

  printf("Test complete: %s\n",__FUNCTION__);
}


void test_eqep_2() {
  printf("Running test: %s\n",__FUNCTION__);

  extern int fd_eqep_position;

  setup();
  char b[10];
  int ii = 0;
  for( ii=0; ii<5; ii++ ) {
    int i = pread(fd_eqep_position,b,10,0);
    printf("read returned: %d (%s)\n",i, strerror(errno));
    printf("b: %s\n",b);
    int j;
    for( j=0; j<i; j++ ) {
      printf("byte %d: %d\n",j,b[j]);
    }
    sleep(1);
  }
  shutdown();
  printf("Test complete: %s\n",__FUNCTION__);
}




void test_motor_dir_pin() {
  printf("Running test: %s\n",__FUNCTION__);

  extern int fd_gpio_dir_direction;
  extern int fd_gpio_dir_value;


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
  printf("Test complete: %s\n",__FUNCTION__);
}


void test_motor_dir_pin2() {
  printf("Running test: %s\n",__FUNCTION__);
  printf("(Test motor dir pin, with fn calls.)\n");
  
  extern int fd_gpio_dir_direction;

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
  printf("Test complete: %s\n",__FUNCTION__);
}

void test_stby_pin() {
  printf("Running test: %s\n",__FUNCTION__);
  
  extern int  fd_gpio_stby_direction;

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
  printf("Test complete: %s\n",__FUNCTION__);
}


void test_rawduty() {
  printf("Running test: %s\n",__FUNCTION__);
  printf("(Test PWM duty cycle.)\n");
  
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
  printf("Test complete: %s\n",__FUNCTION__);
}


void test_rawduty2() {
  printf("Running test: %s\n",__FUNCTION__);
  printf("(Test PWM duty cycle.)\n");
  
  setup();
  unstby();
  run();
  
  cw();
  rawduty("20000",5);
  sleep(1);
  ccw();
  rawduty("15000",5);
  sleep(1);
  cw();
  rawduty("10000",5);
  sleep(1);
  ccw();
  rawduty("5000",4);
  sleep(1);
  cw();
  rawduty("1000",4);
  sleep(1);
  ccw();
  rawduty("0",1);
  sleep(1);
  cw();
  rawduty("5000",4);
  sleep(1);
  ccw();
  rawduty("10000",5);
  sleep(1);
  cw();
  rawduty("15000",5);
  sleep(1);
  ccw();
  rawduty("20000",5);
  sleep(1);
  stop();
  printf("Test complete: %s\n",__FUNCTION__);
}

void test_duty() {
  printf("Running test: %s\n",__FUNCTION__);

  setup();

  unstby();
  run();

  int duties[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0};
  int n = 11;
  int i=0;
  for( i=0; i<n; i++ ) {
    printf("i=%d / %d\n",i,n);
    if( i%2==0 ) cw();
    else         ccw();
    duty(duties[i]);
    sleep(1);
  }

  stop();
  printf("Test complete: %s\n",__FUNCTION__);
}




void test_duty_sine() {

  printf("Running test: %s\n",__FUNCTION__);
  printf("Test duty() with doubles: change the duty cycle sinusoidally.\n");

  setup();
  unstby();
  run();
  cw();

  double dt = 0.1; // sec, time per iteration
  double max_time = 5; // sec, max time of sim
  int num_iters = max_time / dt;
  double freq = 1; // Hz, controls how fast the duty cycle changes

  int i=0;
  for( i=0; i<num_iters; i++ ) {

    double d = 50.0 + 50.0 * sin(2.0 * M_PI * freq * dt * i);
    duty(d);
    usleep(dt*1000000.0);
  }

  stop();
  shutdown();
  printf("Test complete: %s\n",__FUNCTION__);
}
  


