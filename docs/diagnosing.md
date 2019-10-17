# Diagnosing pi-fan problems

## 1: Running in the foreground
Pi-fan normally runs in the background as a daemon. But to verify that the pi-fan executable can actually run and does what it is supposed to do, it can be asked to run in the foreground.

(Note: If you have already installed pi-fan and hooked into systemd, you must use the command ``systemctl stop pi-fan`` to stop a running instance of pi-fan before you try to run pi-fan manually).

To run pi-fan in the foreground use the -f (or --foreground) flag:
```
# pi-fan -f
Running in foreground .....
Setting fan = 35 for temp = 34.563000
Timeout, restarting loop
Setting fan = 34 for temp = 34.270802
Timeout, restarting loop
Setting fan = 33 for temp = 34.173401
Timeout, restarting loop
Setting fan = 33 for temp = 34.173401
...

```
This causes pi-fan to write some text each time it does something. It lists the temperature it is seeing (in degrees C) and the fan speed that is sets (in percent from 0-100).

Apart from showing what pi-fan sees and does, it's also a great way to experiment with settings.

Note that you should run pi-fan with root permissions, otherwise it is unable to use the GPIO pins.

## Sending a signal (SIGHUP) to pi-fan
If you already have a pi-fan instance running (it doesn't matter if you have started it manually or via systemd), you can make it do a couple of "ramp-the-fan-up-and-down-again" cycles by sending it a signal. It provides a simple and audible feedback mechanism that shows that pi-fan is alive and in control of the fan.

First find pid (process id) of pi-fan:
```
# ps -e | grep pi-fan
16244 ?        00:00:00 pi-fan
```
Okay, so pid is 16244, send send a SIGHUP (signal 1 ) to pi-fan:
```
# kill -1 16244
```

If you you're close enough to your Pi to see and hear it, you'll hear that pi-fan runs the fan speed form 0% up 100% and back again a few times. Once it's done, it will resume normal operation.
