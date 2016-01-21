. /etc/modems/modem.path

# For ppp modems the metrics are added in /lib/netifd/ppp-up
# We can later move it to common path

# metric	logical name		physical name
# ----------------------------------------------------------------------
# x		SFP(x)			eth(y)/sfp(y)
# 10+x		INTERNET(x)		eth(y)/internet(y)
# 		GE(x)			eth(y)/ge(y)
# 		pppoe-INTERNET(x)	pppoe-INTERNET(x)
# 		pppoe-GE(x)		pppoe-GE(x)
# 50+x		USB(x)			eth(y) / wwan(y) / USB(x)
# 100+x		other 			?

get_route_metric() {
	local INTERFACE WAN_PREFIXES DEFAULT_METRIC

	# The offsets for the route metrics for different interface prefixes
	# Give priority to SFP if it exists. Leave a gap for pppoe!
	# WARNING: Match this to the logic in mgd/device/network.py
	WAN_PREFIXES="SFP:0 INTERNET:10 GE:10 $modem_string:50"
	DEFAULT_METRIC=100

	INTERFACE="$1"
	if [ "${INTERFACE//pppoe-}" != "${INTERFACE}" ]; then
		INTERFACE="${INTERFACE//pppoe-}"
	fi
	IPFX=${INTERFACE%%[0-9]*}
	INUM=${INTERFACE//$IPFX}
	: ${INUM:=0}
	for P in $WAN_PREFIXES; do
		PPFX=${P%%:*}
		PMET=${P//*:}
		if [ "${PPFX}" = "${IPFX}" ]; then
			echo $(( $PMET + $INUM ))
			return
		fi
	done
	echo $(( $DEFAULT_METRIC + $INUM ))
}


# Local Variables:
# sh-basic-offset: 8
# indent-tabs-mode: t
# End:
# vim: set ts=8 noexpandtab sw=8:
