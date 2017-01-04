#include <stdio.h> // atof
#include <stdlib.h> // atof
// http://stackoverflow.com/questions/20787235/atof-is-returning-ambiguous-value

// these tests use the bb-simple-sysfs-c-lib
extern void test_eqep() ;
extern void test_voltage_sine() ;
extern void test_pid ( double kp ) ;

// these tests don't use the lib; I built them gradually as I
// developed the lib; they probably won't work with the library, but they
// are good documentation of what I did.
extern void test_eqep_1() ;
extern void test_eqep_2() ;
extern void test_motor_dir_pin() ;
extern void test_motor_dir_pin2() ;
extern void test_stby_pin() ;
extern void test_rawduty() ;
extern void test_rawduty2() ;
extern void test_duty() ;
extern void test_duty_sine() ;


int main ( int argc, char *argv[] ) {

  test_eqep();
  test_voltage_sine();

  double kp = -.02;
  if( argc > 1 ) {
    printf("input: %s\n",argv[1]);
    kp = atof(argv[1]);
  }
  test_pid( kp );
}
