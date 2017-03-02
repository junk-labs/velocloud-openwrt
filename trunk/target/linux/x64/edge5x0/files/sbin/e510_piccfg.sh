#!/bin/sh

BOARD=`cat /sys/devices/platform/vc/board 2>/dev/null`
if [ "${BOARD//edge510}" = "${BOARD}" ]; then
	# not a 510
        1>&2 echo "Not an edge 510"
        exit 1
fi

# Load the i2c-dev driver if not already loaded
(lsmod | grep -q i2c_dev) || modprobe i2c-dev

i2cread() {
	i2cget -y "$@"
}

i2creadbyte() {
	OUT=`i2cread 1 "$@"`
        echo $((OUT))
}

i2creadword() {
	OUT=`i2cread 1 "$@" 0 w`
        echo $((OUT))
}

i2creadstring() {
        # only 2-byte strings
	OUT=`i2cread 1 "$@" 0 w`
        printf "\x${OUT:4:2}\x${OUT:2:2}"
}

i2cwrite() {
	i2cset -y 1 "$@"
}

log_sys_restart_reason() {
	case "X$1" in
	    X1) MSG="System restarted via warm reset or SOC watchdog (1)";;
	    X2) MSG="System restarted via cold reset (2)";;
	    X3) MSG="System started up from powered-off state (3)";;
	    X4) MSG="System restarted with PIC power cycle (4)";;
	    X5) MSG="System restarted by PIC watchdog (5)";;
	    X6) MSG="System restarted by reset button push (6)";;
	    X7) MSG="System restarted by thermal trip (7)";;
	    X8) MSG="System restarted by loss of power-good signal (8)";;
	    X9) MSG="System restarted by loss of PMU_SLP (9)";;
	    X10) MSG="System restarted with PIC power cycle on cold reset (10)";;
	    *) MSG="System restarted for unknown reason ($1)" ;;
	esac
	echo "$MSG" > /dev/kmsg
}

bit_set() {
	V=$(($1))
        [[ $((V&(1<<$2))) -ne 0 ]]
}

log_pic_reset_reason() {
        REASONS=""
        R=$(($1))
        if bit_set $R 0; then
        	REASONS="$REASONS, power-on reset"
        fi
        if bit_set $R 1; then
        	REASONS="$REASONS, brown-out reset"
        fi
        if bit_set $R 2; then
        	REASONS="$REASONS, PIC watchdog reset"
        fi
        if bit_set $R 3; then
        	REASONS="$REASONS, MCLR pin reset"
        fi
        if bit_set $R 4; then
        	REASONS="$REASONS, reset instruction"
        fi
        if bit_set $R 5; then
        	REASONS="$REASONS, stack over/underflow"
        fi
        REASONS=${REASONS/, }
        if [ ! -z "$REASONS" ]; then
		echo "PIC restart reason(s): $REASONS" > /dev/kmsg
	fi
}

# Reset the PIC watchdog for now, until we can poke it regularly
OUT=`i2creadword 0x22`
echo "PIC watchdog timer value before reset: $OUT" > /dev/kmsg
i2cwrite 0x22 0 0

# Set the PIC restart type to power-cycle on cold reset
# i2cwrite 0x21 10

# Print the PIC version
OUT=`i2creadstring 0x28`
echo "PIC version $OUT" > /dev/kmsg

# Print the system restart reason
OUT=`i2creadbyte 0x21`
log_sys_restart_reason $OUT

# Print the PIC reset reason
OUT=`i2creadbyte 0x29`
log_pic_reset_reason $OUT

# Read the push-button press time, if any
OUT=`i2creadword 0x2a`
if [ "$OUT" != "0" ]; then
	OUT=`echo $OUT | awk '{printf("%0.2f", $1/100.0);}'`
	echo "PIC push-button press time $OUT" > /dev/kmsg
fi

# unload i2c-dev?
# rmmod i2c-dev
