# Installing pi-fan

_(This is work in progress, more info is the the way ...)_

The short version (which assumes you have a compiled version of pi-fan) is listed below. Note that installing pi-fan requires root-permissions.

1. Copy pi-fan to your Pi, e.g. to /usr/local/bin or whatever your fancy may be. 
2. Copy the systemd service unit (``systemd/pi-fan.service``) to ``/etc/systemd/system/`` on your Pi.
3. Modify the line ``ExecStart=/usr/local/bin/pi-fan`` in the service unit file to match the spot where you placed pi-fan.
4. Run the command ``systemctl start pi-fan`` from the command line (still as root)
   
That's it. Pi-fan shouldrun, and your fan start spinning. Pi-fan will automatically start when you (re-) boot your Pi.

You can start and stop pi-fan vi systemd using the commands
```
-- Run these commands with root permissions --
# systemctl start pi-fan
# systemctl stop pi-fan
```