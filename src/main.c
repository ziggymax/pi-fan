
#include "bcm2835.h"
#include "utils.h"

#include <argp.h>
#include <stdio.h>
#include <unistd.h>

// Setup parameter block with default values. They can changed via command line parameters.
static setup_params_t setup_parameters = {
    /* int sampleperiod */ 2,  // 1) Sample period (seconds between sampling temperature)
    /* int avgsamples   */ 5,  // 2) Number of temperature samples to use for avg. temperature
    /* int minlevel     */ 20, // 3) Minimum fan level - fan never goes below this level
    /* int mintemp      */ 30, // 4) Temperature to be reached before fan level is increased
    /* int maxtemp      */ 55, // 5) Temperature at which fan level is set at 100% duty cycle
    /* int exitlevel    */ 60  // 6) Fan level to set before program exits
};

// Other program parameters
static int run_in_foreground = 0;

// Command line parsing stuff
const char *argp_program_version     = "fan-control v1.0";
const char *argp_program_bug_address = 0;

const char *pid_file = "/run/pi-fan.pid";

static char doc[] =
    "pi-fan is a small server that controls fan speed based on the core temperature. "
    "It uses a GPIO pin to generate a 24KHz PWM signal that can control (almost) any fan "
    "that has a PWM control input. Valid ranges and default values for parameters are "
    "shown as [<from-to>/<default>]."
    "\nExample: [1-30/2] means that legal values are 1 to 30 (inclusive) and the default "
    "value is 2."
    "\nNOTE: If you send a running pi-fan instance a SIGHUP signal, it will run the fan in "
    "a test pattern (varying fan from 0-100%) for a few cycles. You can use this for test or "
    "demo purposes.";

// Known program options
static struct argp_option options[] = {
    {"sampleper", 'p', "SECONDS", 0, "Sample period in secs [1-30/2]", 0},
    {"avgsample", 'g', "SAMPLES", 0, "Samples for average temp [1-10/5]", 0},
    {"minfanlevel", 'l', "MIN_FAN", 0, "Min. fan level (fan will never go lower) [0-100/20]",
     0},
    {"mintemp", 'i', "MIN_TEMP", 0, "Min. temp before fan level increases [0-100/30]", 0},
    {"maxtemp", 'a', "MAX_TEMP", 0, "Temp where fan level is set at max (100%) [0-100/55]", 0},
    {"exitlevel", 'e', "EX_LEVEL", 0, "Level set for fan at program exit [0-100/50]", 0},
    {"foreground", 'f', 0, 0, "Run program in foreground (for diagnostics)", 0},
    {0}};

// Option parser function
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  int paramvalue;

  switch (key) {
  case 'p':
    if ((paramvalue = parse_uint(arg, 1, 30)) < 0)
      argp_error(state, "Invalid value for sample period '%s'", arg);
    setup_parameters.sampleperiod = paramvalue;
    break;
  case 'g':
    if ((paramvalue = parse_uint(arg, 1, 10)) < 0)
      argp_error(state, "Invalid value # samples used for avg temp '%s'", arg);
    setup_parameters.avgsamples = paramvalue;
    break;
  case 'l':
    if ((paramvalue = parse_uint(arg, 0, 100)) < 0)
      argp_error(state, "Invalid value for minimum fan level '%s'", arg);
    setup_parameters.minlevel = paramvalue;
    break;
  case 'i':
    if ((paramvalue = parse_uint(arg, 0, 100)) < 0)
      argp_error(state, "Invalid value for minimum temperature '%s'", arg);
    setup_parameters.mintemp = paramvalue;
    break;
  case 'a':
    if ((paramvalue = parse_uint(arg, 0, 100)) < 0)
      argp_error(state, "Invalid value for maximum temperature '%s'", arg);
    setup_parameters.maxtemp = paramvalue;
    break;
  case 'e':
    if ((paramvalue = parse_uint(arg, 0, 100)) < 0)
      argp_error(state, "Invalid value for exit fan level '%s'", arg);
    setup_parameters.exitlevel = paramvalue;
    break;
  case 'f':
    run_in_foreground = 1;
    break;
  case ARGP_KEY_END:
    if (setup_parameters.mintemp >= setup_parameters.maxtemp)
      argp_error(state, "Minimum temperature must be smaller than maximum temperature");
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, 0, doc, 0, 0, 0};

int main(int argc, char *argv[]) {
  argp_parse(&argp, argc, argv, 0, 0, 0);

  if (run_in_foreground && !create_pidfile(pid_file)) {
    printf("Could not grab pidfile (another instance running?), exiting\n");
    return 1;
  }

  if (!run_in_foreground) {
    if (daemonize() != 0) {
      printf("Could not daemonize, exiting\n");
      return 1;
    }
    if (!create_pidfile(pid_file)) {
      printf("Could not grab pidfile (another instance running?), exiting\n");
      return 1;
    }
  } else
    printf("Running in foreground .....\n");

  int carry_on = 1;

  if (!initialize(&setup_parameters)) {
    printf("Could not initialize gpio/fan settings or setup signal handling, exiting\n");
    return 1;
  }

  do {
    curr_values_t curr_val = update_levels();

    if (isatty(STDOUT_FILENO))
      printf("Setting fan = %d for temp = %f\n", curr_val.fan_level, curr_val.avg_temp);

    switch (get_event()) {
    case EVNT_ERROR:
      if (run_in_foreground) {
        printf("Got error event, terminating\n");
        carry_on = 0;
      } else
        sleep(5);
      break;
    case EVNT_TERMINATE:
      carry_on = 0;
      break;
    case EVNT_TIMEOUT:
      if (run_in_foreground)
        printf("Timeout, restarting loop\n");
      break;
    case EVNT_FANTEST:
      if (run_in_foreground)
        printf("Got SIGHUP, doing fan test\n");
      run_fan_test();
      break;
    }

  } while (carry_on);

  set_exit_fan_level();

  return 0;
}