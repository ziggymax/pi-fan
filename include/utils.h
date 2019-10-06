#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

typedef struct setup_params {
  int sampleperiod; // 1) Sample period (seconds between sampling temperature)
  int avgsamples;   // 2) Number of temperature samples to use for avg. temperature
  int minlevel;     // 3) Minimum fan level
  int mintemp;      // 4) Temperature to be reached before fan level is increased
  int maxtemp;      // 5) Temperature at which fan level is set at 100% duty cycle
  int exitlevel;    // 6) Fan level to set before program exits
} setup_params_t;

typedef struct curr_values {
  int fan_level;
  float avg_temp;
} curr_values_t;

typedef enum { EVNT_ERROR, EVNT_TERMINATE, EVNT_TIMEOUT, EVNT_FANTEST } event_t;

extern int daemonize();
extern int create_pidfile(const char *lockfile);
extern int initialize(setup_params_t *params);
extern int parse_uint(const char *string, int minval, int maxval);
extern curr_values_t update_levels();
extern void set_exit_fan_level();
extern event_t get_event();
extern void run_fan_test();

#endif