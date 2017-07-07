#!/bin/sh

. /etc/modems/modem.path
USB=$1
trigger=$2

log() {
	logger -t "usbmodems" "$@"
	echo "$@"
}

logerr() {
	logger -t "usbmodems" "error: $@"
	echo "$@" 1>&2
}

get_bridge_interface() {
	echo $(uci -X show network | \
		      awk -F '=|\.' '/network.network[0-9]*=interface/ { print "br-"$2}')
}

get_usb_ifname()
{
	local usb=$1
	# avoid uci stderr
	echo $(uci -c $modem_config_path get modems.$usb.ifname 2>/dev/null)
}

get_usb_type()
{
	local usb=$1
	# avoid uci stderr
	echo $(uci -c $modem_config_path get modems.$usb.type 2>/dev/null)
}

get_all_available_usb()
{
	echo $(uci -c $modem_config_path show modems 2>/dev/null | \
		awk -F'=|\.' '/modems.*=interface/ {print $2}')
}

get_edge_activatedstate()
{
	local pid=$(pgrep edged)
	if [ ! -z $pid ]; then
	# edged running
		if [ -f "/opt/vc/.edge.info" ]; then
			echo 1
			return
		fi
	fi
	echo 0
}

modems_forward_rule()
{
	local inInterface=$1
	local outInterface=$2
	local action=$3
	local rule=$4
	iptables -$action VCMP_FWD_ACL -i $inInterface -o $outInterface -j $rule
}

modems_postrouting_rule()
{
	local outInterface=$1
	local action=$2
	#iptables -tnat -$action POSTROUTING -o $outInterface -j MASQUERADE
}

modems_fwd_bridge_usblist()
{
	local usblist=$1
	local insert_delete_rule=$2
	local accept_drop_action=$3
	local usb usbifname
	local brintf brinterfaces
	if [ ! -n "$usblist" ];
	then
		return
	fi
	brinterfaces=$(get_bridge_interface)
	for usb in $usblist
	do
		usbifname=$(get_usb_ifname $usb)
		for brintf in $brinterfaces
		do
			modems_forward_rule $brintf $usbifname $insert_delete_rule $accept_drop_action
		done
	done
}

modems_nat_usblist()
{
	local usblist=$1
	local action=$2
	local usb usbifname
	if [ ! -n "$usblist" ];
	then
		return
	fi
	for usb in $usblist
	do
		usbifname=$(get_usb_ifname $usb)
		modems_postrouting_rule "$usbifname" "$action"
	done
}

modems_plugin_add_rule()
{
	local usb=$1
	local edgeactivated=$(get_edge_activatedstate)
	if [ "$edgeactivated" == "1" ]; then
		modems_fwd_bridge_usblist "$usb" "D" "ACCEPT"
		modems_fwd_bridge_usblist "$usb" "I" "DROP"
	else
		modems_fwd_bridge_usblist "$usb" "D" "DROP"
		modems_fwd_bridge_usblist "$usb" "I" "ACCEPT"
	fi
	modems_nat_usblist "$usb" "I"
}

modems_plugout_delete_rule()
{
	local usb=$1
	# ignore errors when deleting, e.g. if we call multiple times
	modems_fwd_bridge_usblist "$usb" "D" "ACCEPT" 2>/dev/null
	modems_fwd_bridge_usblist "$usb" "D" "DROP" 2>/dev/null
	modems_nat_usblist "$usb" "D" 2>/dev/null
}

modems_edged_run_add_rule()
{
	local usball=$(get_all_available_usb)
	modems_fwd_bridge_usblist "$usball" "D" "ACCEPT"
	modems_fwd_bridge_usblist "$usball" "D" "DROP"
	modems_fwd_bridge_usblist "$usball" "I" "DROP"

	#configure IPtables to permit making REST API calls to IP modems
	iptables -D VCMP_IN_ACL --source 192.168.32.2 -j ACCEPT
	iptables -I VCMP_IN_ACL --source 192.168.32.2 -j ACCEPT
	iptables -D VCMP_IN_ACL --source 192.168.14.1 -j ACCEPT
	iptables -I VCMP_IN_ACL --source 192.168.14.1 -j ACCEPT
	# edged also sucks in the modem http response packets and spits it
	# out on vce1, the pantech modems specifically are interface-bound
	# via curl --interface option, so a tcp packet with the same tuple
	# but on a different interface (vce1) will send a RST to the peer
	iptables -I INPUT -i vce1 -s 192.168.32.2 -p tcp --sport 80 -j DROP
}

modems_edged_kill_delete_rule()
{
	local usball=$(get_all_available_usb)
	modems_fwd_bridge_usblist "$usball" "D" "ACCEPT"
	modems_fwd_bridge_usblist "$usball" "D" "DROP"
	modems_fwd_bridge_usblist "$usball" "I" "ACCEPT"
	#Remove IPtable rules that was permiting REST API calls to IP modems
	iptables -D VCMP_IN_ACL --source 192.168.32.2 -j ACCEPT
	iptables -D VCMP_IN_ACL --source 192.168.14.1 -j ACCEPT
	iptables -D INPUT -i vce1 -s 192.168.32.2 -p tcp --sport 80 -j DROP
}

modems_insert_fw_allusb()
{
	local usball=$(get_all_available_usb)
	modems_plugin_add_rule "$usball"
}

modems_delete_fw_allusb()
{
	local usball=$(get_all_available_usb)
	modems_plugout_delete_rule "$usball"
}

load_modem_status()
{
	# Load modem status:
	#   UNAVAILABLE
	#   RUNNING
	#   STOPPED
	#   UNKNOWN

	# Don't go on if type is empty (i.e. no config for USB#)
	TYPE=$(get_usb_type $USB)
	if [ -z "$TYPE" ]; then
		logerr "$USB: Unknown modem type, cannot run modem service operation"
		exit 1
	fi

	# For ModemManager managed modems, we don't check ifname, as ifname is only
	# set once the modem has been connected.
	if [ "$TYPE" != "modemmanager" ]; then
		$(uci -c $modem_config_path get modems.$USB.ifname >/dev/null 2>&1)
		if [ $? != 0 ]; then
			STATUS="UNAVAILABLE"
			return
		fi
	fi

	# Check if the PID file exists
	if [ -f /$tmpdir/$USB.pid ]; then
		# kill -0 checks whether the process is running
		local pid=$(cat /$tmpdir/$USB.pid)

		if [ -z $pid ]; then
			STATUS="UNKNOWN"
			return
		fi

		kill -0 $pid
		if [ $? -eq 0 ]; then
			STATUS="RUNNING"
			return
		fi
	fi

	STATUS="STOPPED"
}

modem_start()
{
	log "$USB: starting..."

	# Avoid starting if already running
	load_modem_status
	if [ "$STATUS" == "RUNNING" ]; then
		logerr "$USB: already running, cannot start"
		exit 1
	fi
	if [ "$STATUS" == "UNAVAILABLE" ]; then
		logerr "$USB: modem not detected, cannot start"
		exit 1
	fi

	if [ "$TYPE" == "modemmanager" ]; then
		log "$USB: starting ModemManager support"
		nohup $modem_script_path/MMModems/mm_run.sh $USB 2>&1 >/dev/null &
	elif [ "$TYPE" == "serial" ]; then
		log "$USB: starting PPP modem support"
		nohup $modem_script_path/TTYModems/ppp_run.sh -ifname $USB 2>&1 >/dev/null &
	else
		log "$USB: starting WWAN modem support"
		nohup $modem_script_path/IPModems/IPModemsRun.py $USB 2>&1 >/dev/null &
	fi

	modems_plugin_add_rule "$USB"
	log "$USB: started modem $TYPE script sucessfully"

	uci -c $modem_config_path set modems.$USB.started=''"1"''
}

modem_stop()
{
	log "$USB: stopping..."

	load_modem_status
	if [ "$STATUS" == "UNAVAILABLE" ]; then
		logerr "$USB: modem not detected, cannot stop"
		exit 1
	fi

	if [ -f /$tmpdir/$USB.pid ]; then
		local pid=$(cat /$tmpdir/$USB.pid)
		if [ -z $pid ]; then
			logerr "$USB: empty pid file found"
			rm -f /$tmpdir/$USB.pid
		else
			# Check if process is running
			kill -0 $pid > /dev/null 2>&1
			if [ $? -ne 0 ]; then
				# Already stopped, good!
				log "$USB: process with pid $pid is already stopped"
				rm -f /$tmpdir/$USB.pid
			else
				# Retry to kill up to 10 times
				local count=0

				while [ $count -le 10 ]; do
					# 0,1,2,3. SIGTERM
					if [ $count -le 3 ]; then
						log "$USB: stopping process with pid $pid (SIGTERM)"
						kill -SIGTERM $pid
					# 4,5,6, SIGINT
					elif [ $count -le 6 ]; then
						log "$USB: stopping process with pid $pid (SIGINT)"
						kill -SIGINT $pid
					# 7,8,9, SIGHUP
					elif [ $count -le 9 ]; then
						log "$USB: stopping process with pid $pid (SIGHUP)"
						kill -SIGHUP $pid
					# 10, SIGKILL
					else
						log "$USB: stopping process with pid $pid (SIGKILL)"
						kill -SIGKILL $pid
					fi

					sleep 1

					kill -0 $pid > /dev/null 2>&1
					if [ $? -ne 0 ]; then
						# Already stopped, good!
						log "$USB: process with pid $pid now stopped"
						rm -f /$tmpdir/$USB.pid
						break
					fi

					# Still running
					count=$((count + 1))
				done
			fi
		fi
	else
		log "$USB: already stopped"
	fi

	modems_plugout_delete_rule "$USB"

	# If still not flagged as stopped, return error
	load_modem_status
	if [ "$STATUS" != "STOPPED" ]; then
		logerr "$USB: couldn't stop"
		exit 1
	fi

	uci -c $modem_config_path set modems.$USB.started=''"0"''
}

modem_restart()
{
	log "$USB: restarting..."
	modem_stop
	sleep 5
	modem_start
	log "$USB: restarted."
}

modem_status()
{
	load_modem_status
	case "$STATUS" in
		"RUNNING")
			log "$USB: modem script running"
			;;
		"STOPPED")
			log "$USB: modem script not running"
			;;
		"UNAVAILABLE")
			log "$USB: modem not found"
			exit 1
			;;
		*)
			log "$USB: unknown status"
			exit 2
			;;
	esac
}

case "$trigger" in
	'start')
		modem_start
	  ;;
	'stop')
		modem_stop
	  ;;
	'restart')
		modem_restart
	  ;;
	'status')
		modem_status
	  ;;
	'edged')
		modems_edged_run_add_rule
	  ;;
	'noedged')
		modems_edged_kill_delete_rule
	  ;;
	'fwrestart')
		modems_insert_fw_allusb
	  ;;
	*)
		echo "Usage: $0 USB<number> start|stop|restart"
		echo "	     $0 usball edged|noedged|fwrestart"
esac
