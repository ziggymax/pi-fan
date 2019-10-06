# pi-fan
A small server that can control a fan on a Raspberry Pi based on CPU temperature. For now I have only tested if this works with a Raspberry Pi 4. Pi-fan works by controlling a fan with a standard 4-pin PWM (25KHz) input. I personally use a small Noctua 40x10 mm 5V fan (NF-A4x10) which works as a charm. It's small, quiet and effective.

# Some high-lights
Pi-fan uses hardware pwm and hence uses very few cpu ressources. Pi-fan is intended to be run  via systemd, so that it starts and stops automatically. There's a small systemd service unit ready-for-use in the folder systemd.

Pi-fan allows you to configure the parameters listed below. While there's ample opportunity to tweak around, pi-fan works very well with the default values (listed in parenthesis)
1. Core temperature sample period, i.e. the interval between measurement of the core temperature (2 seconds).
2. How many temperature samples should be used to form the running average temperature which is used to control the fan (5).
3. **MinFan**: The lowest fan level to use (fan speed will never go below this value) (20%).
4. **MinTemp**: The minimum temperature at which pi-fan starts to increase the fan speed (30 degrees Celcius).
5. **MaxTemp**: The temperature at which the fan speed should reach 100% (55 degrees Celcius).
6. The fan level to set if pi-fan is asked to stop (50%).

This figure illustrates some of these parameters:

![alt text](images/regulation.png "Pi-fan regulation")

So the fan speed never goes under MinFan. Once the temperature reaches MinTemp, pi-fan starts to ramp up the fan speed until it reaches 100% at MaxTemp. The fan ramp-up is linear.

# Build instructions
More will be forthcoming, but for now you need to know (at least) this: pi-fan uses Mike McCauley's excellent little Pi gpio library called bcm2835 (link: https://www.airspayce.com/mikem/bcm2835/index.html). For now pi-fan compiles/links bcm2835 statically into the binary, and you need to grab bcm2835 from the link, and place the two files `bcm2835.h` and `bcm2835.c` into the `include/` and `src/` folders, respectively. The CMake build assumes that these 2 files can be found in the folders as described.


