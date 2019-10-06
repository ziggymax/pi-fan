#include "bcm2835.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// --- GPIO configuration ---
// PIN to use for PWM output
#define GPIO_PIN RPI_GPIO_P1_12
// PWM channel to use
#define PWM_CHANNEL 0
// PWM range
#define PWM_RANGE 100

static setup_params_t *setup_parameters;

static int signal_fd;

static int setup_signals() {
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGTSTP);
  sigdelset(&mask, SIGCONT);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) != 0)
    return -1;

  return signal_fd = signalfd(-1, &mask, SFD_CLOEXEC);
}

int initialize(setup_params_t *params) {
  setup_parameters = params;

  if (!bcm2835_init())
    return 0;

  bcm2835_gpio_fsel(GPIO_PIN, BCM2835_GPIO_FSEL_ALT5);
  bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_8);
  bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
  bcm2835_pwm_set_range(PWM_CHANNEL, PWM_RANGE);

  return setup_signals();
}

event_t get_event() {
  static const int MAXFD = 1;
  static int initialized = 0;
  static int epoll_fd    = -1;
  struct epoll_event events[MAXFD];

  if (!initialized) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
      return EVNT_ERROR;

    struct epoll_event epe;
    memset(&epe, 0, sizeof(epe));
    epe.events  = EPOLLIN;
    epe.data.fd = signal_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &epe) < 0)
      return EVNT_ERROR;

    initialized = 1;
  }

  do {
    int numevents = epoll_wait(epoll_fd, events, MAXFD, setup_parameters->sampleperiod * 1000);

    if (numevents < 0)
      return EVNT_ERROR;
    if (numevents == 0)
      return EVNT_TIMEOUT;

    struct signalfd_siginfo fdsiginfo;
    ssize_t s = read(signal_fd, &fdsiginfo, sizeof(fdsiginfo));

    if (s == sizeof(fdsiginfo)) {
      int signal = fdsiginfo.ssi_signo;
      switch (signal) {
      case SIGTERM:
      case SIGINT:
        return EVNT_TERMINATE;
        break;
      case SIGHUP:
        return EVNT_FANTEST;
        break;
      default:
        break;
      }
    }
  } while (1);

  return EVNT_ERROR;
}

#define BD_MAX_CLOSE 8192

int daemonize() {

  switch (fork()) {
  case -1:
    return -1;
  case 0:
    break;
  default:
    _exit(EXIT_SUCCESS);
  }
  if (setsid() == -1)
    return -1;
  switch (fork()) {
  case -1:
    return -1;
  case 0:
    break;
  default:
    _exit(EXIT_SUCCESS);
  }

  umask(0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  chdir("/");
#pragma GCC diagnostic pop

  int close_upper_limit = sysconf(_SC_OPEN_MAX);
  if (close_upper_limit == -1)
    close_upper_limit = BD_MAX_CLOSE;
  for (int open_fd = 0; open_fd < close_upper_limit; open_fd++) close(open_fd);

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  int open_fd = open("/dev/null", O_RDWR);
  if (open_fd != STDIN_FILENO)
    return -1;
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
    return -1;
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
    return -1;
  return 0;
}

int create_pidfile(const char *pidfile) {
  int pidfile_fd = -1;

  if ((pidfile_fd = open(pidfile, O_RDWR | O_CREAT | O_CLOEXEC | O_EXCL, S_IWUSR | S_IRUSR)) !=
      -1) {
    const int BUFSIZE = 20;
    char pidinfo[BUFSIZE];
    int len = snprintf(pidinfo, BUFSIZE, "%d\n", getpid());
    if (write(pidfile_fd, pidinfo, len) == len)
      return 1;
    close(pidfile_fd);
    return 0;
  }

  if ((pidfile_fd = open(pidfile, O_RDWR | O_CLOEXEC)) != -1) {
    const int BUFSIZE = 20;
    char buf[BUFSIZE + 1];
    int numbytes = read(pidfile_fd, buf, BUFSIZE);
    if (numbytes > 0) {
      buf[numbytes + 1] = '\0';
      int oldpid        = atoi(buf);
      if (oldpid != 0 && kill(oldpid, 0) == -1 && errno == ESRCH) {
        if (remove(pidfile) == 0)
          return create_pidfile(pidfile);
      }
    }
  }

  return 0;
}

int parse_uint(const char *string, int minval, int maxval) {
  errno   = 0;
  int res = strtol(string, NULL, 10);

  if (errno != 0 || res < minval || res > maxval)
    return -1;

  return res;
}

static int get_curr_temp() {
  int retval       = -1;
  const int bufsiz = 100;
  char buffer[bufsiz + 1];
  ssize_t readbytes = 0;

  int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);

  if (fd <= 0)
    goto done;

  readbytes = read(fd, buffer, bufsiz);

  if (readbytes <= 0)
    goto done;

  retval = atoi(buffer);

done:
  if (fd > 0)
    close(fd);
  return retval;
}

static float get_avg_temp() {
  static int initialized = 0;
  static int *values     = NULL;
  static int index       = 0;

  int currtemp = get_curr_temp();

  if (!initialized) {
    values = (int *)malloc(setup_parameters->avgsamples * sizeof(int));
    for (int j = 0; j < setup_parameters->avgsamples; j++) *(values + j) = currtemp;
    initialized = 1;
  }

  values[index++] = currtemp;
  if (index >= setup_parameters->avgsamples)
    index = 0;

  int sumtemp = 0;
  for (int j = 0; j < setup_parameters->avgsamples; j++) sumtemp += values[j];

  return sumtemp / (1000.0 * setup_parameters->avgsamples);
}

static int calculate_fan_from_temp(float temp) {
  static int initialized = 0;
  static float factor    = 1.0;

  if (!initialized) {
    factor = (100.0 - setup_parameters->minlevel) /
             (setup_parameters->maxtemp - setup_parameters->mintemp);
    initialized = 1;
  }

  if (temp <= setup_parameters->mintemp)
    return setup_parameters->minlevel;
  if (temp >= setup_parameters->maxtemp)
    return 100;
  return (int)roundf((temp - setup_parameters->mintemp) * factor + setup_parameters->minlevel);
}

void set_exit_fan_level() { bcm2835_pwm_set_data(PWM_CHANNEL, setup_parameters->exitlevel); }

curr_values_t update_levels() {
  curr_values_t curr_val;
  curr_val.avg_temp  = get_avg_temp();
  curr_val.fan_level = calculate_fan_from_temp(curr_val.avg_temp);

  bcm2835_pwm_set_data(PWM_CHANNEL, curr_val.fan_level);

  return curr_val;
}

void run_fan_test() {
  for (int cycle = 0; cycle < 4; cycle++) {
    for (unsigned int fanlevel = 0; fanlevel <= 100; fanlevel++) {
      bcm2835_pwm_set_data(PWM_CHANNEL, fanlevel);
      usleep(25000);
    }
    bcm2835_pwm_set_data(PWM_CHANNEL, 0);
    usleep(750000);
  }
}