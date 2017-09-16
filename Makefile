# Usage: Install Arduino-Makefile and if necessary, update the 'include' path below.
# See https://github.com/sudar/Arduino-Makefile
# Install the libraries listed in ARDUINO_LIBS.

BOARD_TAG    = mega2560
MONITOR_PORT = /dev/ttyACM0
#CFLAGS_STD   = -std=gnu++11

include /usr/share/arduino/Arduino.mk
