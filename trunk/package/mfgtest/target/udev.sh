#!/bin/bash

#set -x
#exec 1> /tmp/udev.1
#exec 2> /tmp/udev.2

#echo Environment:
#env
#echo Arguments:
#echo "$@"

log=/dev/kmsg
SDRDIR=/home/velocloud/bladerf
MAKE=/usr/bin/make

if [ -z "$ACTION" ]; then
	ACTION=$1
fi

case $ACTION in
add)
	#if [ "$DEVTYPE" != "usb_device" ]; then
	#	exit 0
	#fi
	echo "starting SDR server: $ID_SERIAL_SHORT" >> $log
	$MAKE -C $SDRDIR brf-start >> /tmp/sdr 2>&1
	;;
remove)
	#if [ "$DEVTYPE" != "usb_device" ]; then
	#	exit 0
	#fi
	if [ "$ID_MODEL_ID" != "6066" ]; then
		exit 0
	fi
	echo "stopping SDR server: $ID_SERIAL_SHORT" >> $log
	$MAKE -C $SDRDIR brf-stop >> /tmp/sdr 2>&1
	;;
*)
	;;
esac

exit 0

