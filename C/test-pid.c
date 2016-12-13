#include <stdio.h>
#include <unistd.h> // usleep
#include "bb-simple-sysfs-c-lib.h"
#include <math.h>
#include <stdlib.h>

int main ( int argc, char *argv[] ) {

  // Test voltage()

  setup();
  unstby();
  run();
  cw();

  double kp = -.015;
  if( argc > 1 ) {
    printf("input: %s\n",argv[1]);
    kp = atof(argv[1]);
  }

  printf("kp: %lf\n",kp);

  double dt = 0.1; // sec, time per iteration
  double max_time = 10; // sec, max time of sim
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

return 0;
}
