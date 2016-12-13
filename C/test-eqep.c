#include <stdio.h>
#include "bb-simple-sysfs-c-lib.h"

void main() {

  // Test shaft_angle_deg
  setup();
 
  int i=0; 
  for(i=0; i<10; i++) {
    printf("Shaft angle (deg): %lf\n", shaft_angle_deg());
    sleep(1);
  }

  shutdown();

}
