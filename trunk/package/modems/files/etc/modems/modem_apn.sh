#!/bin/sh

if [ -f /etc/modems/modem.path ]; then
	. /etc/modems/modem.path
fi

log()
{
	logger -t "usbmodems|apn" "$@"
	echo "$@"
}

print_usage()
{
	echo ""
	echo "Usage: $PROGRAM [USB] [ACTION] [ACTION ARGS...]"
	echo ""
	echo "  [USB] may be given as [USB<number>] or [MODEM<number>], whatever applies."
	echo ""
	echo "  Supported [ACTION] values are:"
	echo "    check                             Checks whether APN setting is supported."
	echo "    get                               Retrieves current APN settings."
	echo "    set  [apn] [username] [password]  Sets user-provided APN settings."
	echo "    auto [apn] [username] [password]  Sets automatically detected APN settings."
	echo ""
	echo "  Note: user-provided settings always take preference over the automatically set"
	echo "  ones, so once 'set' has been run once, any other 'auto' run will be ignored."
	echo ""
}

get_apn_supported()
{
	# For ModemManager managed modems, we don't check ifname, as ifname is only
	# set once the modem has been connected.
	if [ "$TYPE" != "modemmanager" ]; then
		local ifname=$(uci -c $modem_config_path get modems.$USB.ifname >/dev/null 2>&1)
		if [ -z "$ifname" ]; then
			return
		fi
	fi

	echo $(uci -c $modem_config_path get modems.$USB.apn_supported)
}

run_check()
{
	local supported=$(get_apn_supported)
	if [ "$supported" == "1" ]; then
		log "$USB: APN is supported"
		exit 0
	else
		log "$USB: APN is not supported"
		exit 1
	fi
}

run_get()
{
	local profile=${modem_config_path}/$USB.profile

	if [ ! -f ${profile} ]; then
		log "$USB: profile not set"
		exit 1
	fi

	log "$USB: loading profile: $profile"
	. ${profile}

	echo "  APN:  '$APN'"
	echo "  user: '$APN_USER'"
	echo "  pass: '$APN_PASS'"
	echo "  type: '$APN_TYPE'"
	exit 0
}

run_set()
{
	local profile=${modem_config_path}/$USB.profile

	# Load previous, if any found
	if [ -f ${profile} ]; then
		. ${profile}
	fi

	# If we are trying to set an 'automatic' profile, only allow it if no
	# 'manual' profile has ever been set before.
	if [ "${NEW_APN_TYPE}" == "automatic" ] && [ "${APN_TYPE}" == "manual" ]; then
		log "$USB: avoiding to override manual APN with an automatic one."
		exit 1
	fi

	log "$USB: setting ${NEW_APN_TYPE} APN '${NEW_APN}' user '${NEW_APN_USER}' pass '${NEW_APN_PASS}': $profile"

	mkdir -p $(dirname $profile) >/dev/null 2>&1

	echo "APN=${NEW_APN}"	        >  ${profile}
	echo "APN_USER=${NEW_APN_USER}" >> ${profile}
	echo "APN_PASS=${NEW_APN_PASS}" >> ${profile}
	echo "APN_TYPE=${NEW_APN_TYPE}" >> ${profile}
	# PROXY only applicable to QMI modems
	echo "PROXY=yes" >> ${profile}

	# If modem script running, restart it to use the new APN info
	if [ "$(uci -c /etc/config/modems get modems.$USB.started 2>/dev/null)" == "1" ]; then
		log "$USB: restarting modem service after APN update"
		${modem_script_path}/modem_service.sh $USB restart
	fi

	exit 0
}

PROGRAM=$0
USB=$1
ACTION=$2

if [ "$#" -lt 2 ]; then
	echo "error: missing arguments" 1>&2
	print_usage
	exit 1
fi

# Don't go on if type is empty (i.e. no config for USB#)
TYPE=$(echo $(uci -c $modem_config_path get modems.$USB.type 2>/dev/null))
if [ -z "$TYPE" ]; then
	echo "error: $USB: Unknown modem type, cannot run modem apn operation"
	exit 1
fi

case "$ACTION" in
	'check')
		# No more arguments required
		if [ $? -gt 2 ]; then
			echo "error: too many arguments in 'check' action" 1>&2
			print_usage
			exit 1
		fi

		run_check
		;;
	'get')
		# No more arguments required
		if [ $? -gt 2 ]; then
			echo "error: too many arguments in 'get' action" 1>&2
			print_usage
			exit 1
		fi

		run_get
		;;
	'set')
		# Allows between 2 and 5 arguments
		if [ $? -gt 5 ]; then
			echo "error: too many arguments in 'set' action" 1>&2
			print_usage
			exit 1
		fi

		NEW_APN=$3
		NEW_APN_USER=$4
		NEW_APN_PASS=$5
		NEW_APN_TYPE="manual"
		run_set
		;;
	'auto')
		# Allows between 2 and 5 arguments
		if [ $? -gt 5 ]; then
			echo "error: too many arguments in 'auto' action" 1>&2
			print_usage
			exit 1
		fi

		NEW_APN=$3
		NEW_APN_USER=$4
		NEW_APN_PASS=$5
		NEW_APN_TYPE="automatic"
		run_set
		;;
	*)
		break
esac

echo "error: unknown action '$ACTION'"
print_usage
exit 2
