# _zForth port for ESP8266_

### Configure the project

```
make menuconfig
```

* Set serial port under Serial Flasher Options.  
* `make monitor` baud rate set to what you set in the example.


### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
make -j4 flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.
