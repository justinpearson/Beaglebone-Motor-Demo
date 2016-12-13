#include <stdio.h>
#include <unistd.h> // usleep
#include "bb-simple-sysfs-c-lib.h"
#include <math.h>

void main() {

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
    printf("Shaft angle (deg): %lf\n", shaft_angle_deg());
    double v = MAX_VOLTAGE * sin(2.0 * M_PI * freq * dt * i);
    voltage(v);
    usleep(dt*1000000.0);
  }

  voltage(0);
  stop();
  stby();
  shutdown();

}
