#!/bin/sh

. /etc/modems/modem.path

log()
{
	logger -t "usbmodems|apn" "$@"
	echo "$@"
}

print_usage()
{
	echo "usage: $PROGRAM USB<number> check"
	echo "usage: $PROGRAM USB<number> get"
	echo "usage: $PROGRAM USB<number> set [apn] [username] [password]"
}

get_apn_supported()
{
	local ifname=$(uci -c $modem_config_path get modems.$USB.ifname 2>/dev/null)
	if [ ! -z "$ifname" ]; then
	    echo $(uci -c $modem_config_path get modems.$USB.apn_supported)
	fi
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
	exit 0
}

run_set()
{
	local profile=${modem_config_path}/$USB.profile

	log "$USB: setting APN '$APN' user '$USER' pass '$PASS': $profile"

	mkdir -p $(dirname $profile) >/dev/null 2>&1

	echo "APN=$APN"	      >	 ${profile}
	echo "APN_USER=$USER" >> ${profile}
	echo "APN_PASS=$PASS" >> ${profile}
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

		APN=$3
		USER=$4
		PASS=$5
		run_set
		;;
	*)
		break
esac

echo "error: unknown action '$ACTION'"
print_usage
exit 2
