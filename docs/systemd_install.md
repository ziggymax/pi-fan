# Installing pi-fan

## Hooking into systemd
Assuming you have a working pi-fan executable, you will want to hook it into your system so that pi-fan starts automatically when your Pi boots. The way to do that is to hook pi-fan into systemd. Note that installing pi-fan requires root-permissions.

Here's the steps (all performed as root, either by doing a ``sudo su`` or by running the necessary commands via ``sudo <whatever command>``).

1. Copy pi-fan to a suitable system folder on your Pi, e.g. to /usr/local/bin or whatever your fancy may be. 
2. Copy the systemd service unit (``systemd/pi-fan.service``) to ``/etc/systemd/system/`` on your Pi.
3. Modify the line ``ExecStart=/usr/local/bin/pi-fan`` in the service unit file to match the spot where you placed pi-fan.
4. Run the command ``systemctl start pi-fan`` from the command line (still as root)
   
That's it. Pi-fan should run, and your fan start spinning. Pi-fan will automatically start when you (re-) boot your Pi.

You can start and stop pi-fan via systemd using these commands (with root credentials)
```
-- Run these commands with root permissions --
# systemctl start pi-fan
# systemctl stop pi-fan
```

## Configuring pi-fan
Pi-fan can be configured via a number of command line arguments. Execute ``pi-fan --help`` to see the possible arguments:
```
pi:~ $ pi-fan --help
Usage: pi-fan [OPTION...]
pi-fan is a small server that controls fan speed based on the core temperature.
It uses a GPIO pin to generate a 24KHz PWM signal that can control (almost) any
fan that has a PWM control input. Valid ranges and default values for
parameters are shown as [<from-to>/<default>].
Example: [1-30/2] means that legal values are 1 to 30 (inclusive) and the
default value is 2.
NOTE: If you send a running pi-fan instance a SIGHUP signal, it will run the
fan in a test pattern (from 0-100%) for a few cycles. You can use this for test
or demo purposes.

  -a, --maxtemp=MAX_TEMP     Temp where fan level is set at max (100%)
                             [0-100/55]
  -e, --exitlevel=EX_LEVEL   Level set for fan at program exit [0-100/50]
  -f, --foreground           Run program in foreground (for diagnostics)
  -g, --avgsample=SAMPLES    Samples for average temp [1-10/5]
  -i, --mintemp=MIN_TEMP     Min. temp before fan level increases [0-100/30]
  -l, --minfanlevel=MIN_FAN  Min. fan level (fan will never go lower)
                             [0-100/20]
  -p, --sampleper=SECONDS    Sample period in secs [1-30/2]
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```


## Making your configuration persistent
If you want to make pi-fan use settings that differ from the defaults, you must edit the systemd unit file (``pi-fan.service``). Simply add the parameters to the command line for pi-fan (the line with  ``ExecStart=/<...path...>/pi-fan``).

Example: Say you want to change the fan level that pi-fan sets when it exits, to 70%. So you would change
 ```
# Line in pi-fan.service before
~
ExecStart=/usr/local/bin/pi-fan
~
# is changed into this:
~ 
ExecStart=/usr/local/bin/pi-fan --exitlevel=70
~
```
Restart pi-fan if you make any changes to the .service file:
```
-- Run this commands with root permissions --
# systemctl restart pi-fan
```
