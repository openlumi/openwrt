#!/bin/sh

memtool 20E00E4=15 > /tmp/NULL
memtool 20E00E8=15 > /tmp/NULL


if [ $# != 1 ] && [ $# != 2 ]
then
    echo "Error: Invalid arguments"
    echo "    To flash the firmware without erasing PDM"
    echo "        ${0} <Firmware_File>"
    echo "    To flash the firmware erasing PDM"
    echo "        ${0} <Firmware_File> --erasepdm"
    exit 1
fi

if [ ! -f "$1" ]
then
    echo "Error: Update File $1 does not exist"
    exit 1
fi

# Put JN51xx MISO pin to low
MISO_PIN=40
if [ ! -f "/sys/class/gpio/gpio$MISO_PIN/direction" ]
then 
	echo $MISO_PIN > /sys/class/gpio/export
fi
echo out > /sys/class/gpio/gpio$MISO_PIN/direction
echo 0 > /sys/class/gpio/gpio$MISO_PIN/value

# Now reset JN51xx to put in Programming mode
RESET_PIN_MK=41
if [ ! -f "/sys/class/gpio/gpio$RESET_PIN_MK/direction" ]
then 
	echo $RESET_PIN_MK > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/gpio$RESET_PIN_MK/direction

echo 0 > /sys/class/gpio/gpio$RESET_PIN_MK/value
sleep 1
echo 1 > /sys/class/gpio/gpio$RESET_PIN_MK/value
sleep 1

# Now Flash the firmware
# Call the flasher program
if [ $1 ]
then
  echo Flash $1
  if [ $2 ]
  then
    iot_jp -s /dev/ttymxc1 -f $1 -v -V 2 $2
  else
    iot_jp -s /dev/ttymxc1 -f $1 -v -V 2
  fi
  result=$?
fi
sleep 1

# Put JN51xx MISO pin to High
echo 1 > /sys/class/gpio/gpio$MISO_PIN/value
sleep 1
# Now reset JN51xx to put in running mode
echo 0 > /sys/class/gpio/gpio$RESET_PIN_MK/value
sleep 1
echo 1 > /sys/class/gpio/gpio$RESET_PIN_MK/value
sleep 1

exit 0


