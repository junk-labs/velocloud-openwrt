#!/bin/sh
#
# Program requires a valid USBX or MODEMX id to start
#  e.g. ./mm_run.sh USB3

# Input arguments
if [ "$#" -ne 1 ]; then
    echo "error: illegal number of parameters"
fi
USB=$1

# Global variables
. /etc/modems/modem.path

# Common MM support
. ${modem_script_path}/MMModems/mm_common

# Write PID file of the current process
local PIDFILE="$tmpdir/${USB}.pid"
echo $$ > ${PIDFILE}
if [ $? -ne 0 ]; then
    log_error "couldn't create PID file"
    exit 3
fi

process_modem_list
