#!/bin/sh

. /opt/vc/modems/modem.path
USB=$1
trigger=$2

log() {
	logger -t "usbmodems" "$@"
}

get_bridge_interface() {
	echo $(uci -X show network | \
		      awk -F '=|\.' '/network.network[0-9]*=interface/ { print "br-"$2}')
}

get_usb_ifname() {
	local usb=$1
	echo $(uci -c $modem_path get modems.$usb.ifname)
}

get_usb_type() {
	local usb=$1
	echo $(uci -c $modem_path get modems.$usb.type)
}

get_all_available_usb() {
	echo $(uci -c $modem_path show modems | \
		      awk -F'=|\.' '/modems.*=interface/ {print $2}')
}

get_edge_activatedstate() {
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

modems_forward_rule() {
	local inInterface=$1
	local outInterface=$2
	local action=$3
	local rule=$4
	iptables -$action VCMP_FWD_ACL -i $inInterface -o $outInterface -j $rule
}

modems_postrouting_rule() {
	local outInterface=$1
	local action=$2
	#iptables -tnat -$action POSTROUTING -o $outInterface -j MASQUERADE
}

modems_fwd_bridge_usblist() {
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

modems_nat_usblist() {
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

modems_plugin_add_rule() {
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

modems_plugout_delete_rule() {
	local usb=$1
	modems_fwd_bridge_usblist "$usb" "D" "ACCEPT"
	modems_fwd_bridge_usblist "$usb" "D" "DROP"
	modems_nat_usblist "$usb" "D"
}

modems_edged_run_add_rule() {
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

modems_edged_kill_delete_rule() {
	local usball=$(get_all_available_usb)
	modems_fwd_bridge_usblist "$usball" "D" "ACCEPT"
	modems_fwd_bridge_usblist "$usball" "D" "DROP"
	modems_fwd_bridge_usblist "$usball" "I" "ACCEPT"
	#Remove IPtable rules that was permiting REST API calls to IP modems
	iptables -D VCMP_IN_ACL --source 192.168.32.2 -j ACCEPT
	iptables -D VCMP_IN_ACL --source 192.168.14.1 -j ACCEPT
	iptables -D INPUT -i vce1 -s 192.168.32.2 -p tcp --sport 80 -j DROP
}

modems_insert_fw_allusb() {
	local usball=$(get_all_available_usb)
	modems_plugin_add_rule "$usball"
}

modems_delete_fw_allusb() {
	local usball=$(get_all_available_usb)
	modems_plugout_delete_rule "$usball"
}

modems_start() {
	local type
	modems_stop
	type=$(get_usb_type $USB)
	log "$USB: Start the modem $type script"
	if [ "$type" == "serial" ];then
		nohup $modem_script_path/TTYModems/ppp_run.sh -ifname $USB 2>&1 >/dev/null &
	else
		nohup $modem_script_path/IPModems/IPModemsRun.py $USB 2>&1 >/dev/null &
	fi
	modems_plugin_add_rule "$USB"
	log "$USB: Started modem $type script sucessfully"
}

modems_stop() {
	local pid=$(cat /$tmpdir/$USB.pid)
	local type=$(get_usb_type $USB)
	kill -SIGTERM $pid
	modems_plugout_delete_rule "$USB"
	log "$USB: Kill modem $type script"
}

modems_restart() {
	local type=$(get_usb_type $USB)
	log "$USB: Restarting the modem $type script"
	modems_stop
	sleep 5
	modems_start
	log "$USB: Restart done."
}

modems_status() {
	local type=$(get_usb_type $USB)
	log "$USB: Check for the modem $type status"
	$(uci -c $modem_path get modems.$USB.ifname >/dev/null 2>&1)
	if [ $? != 0 ]; then
		# Modem not detected or not inserted
		echo "None"
		return
	fi
	if [ -f /$tmpdir/$USB.pid ];then
		echo "Running"
	else
		echo "Stopped"
	fi
}

case "$trigger" in
	'start')
		modems_start
	  ;;
	'stop')
		modems_stop
	  ;;
	'restart')
		modems_restart
	  ;;
	'status')
		modems_status
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
