#!/bin/sh

# pppd deamon will be restarted when there is no carrier or when exited due to invalid input
restart_pppd() {
    local apnscript=""
    local dialnumber=""
    local statusvalue=""

    # If modem is 3GPP2, we just dial 777, no APN required
	local cdmagsm=$(uci -c $modem_config_path get modems.$ifname.cdmagsm 2>/dev/null)
    if [ "$cdmagsm" == "CDMA" ]; then
        dialnumber="#777"
        apnscript="3g_noapn.chat"
    elif  [ "$cdmagsm" == "GSM" ]; then
        # If no explicit APN requested, we just default to the one in PDP context #1
        dialnumber="*99***1#"
        if [ -z "$APN" ]; then
            apnscript="3g_noapn.chat"
        else
            apnscript="3g_apn.chat"
        fi
    else
        # Unknown type, we're out
        return
    fi

    # Get dial device
    ttydevice=$(uci -c $modem_config_path get modems.$ifname.device)

    # Start signal strength getter on background
	nohup $modem_script_path/TTYModems/get_sig_strength.sh $ifname 2>&1 >/dev/null &
	sig_strength_pid=$!

    # if file is removed then dont restart the pppd
    while [ -f $pidfile ]; do
        if ! kill -0 $pid 2>&1 > /dev/null;then
            > $chatlog # Empty the log
            nohup /usr/sbin/pppd nodetach ipparam wan ifname $ifname \
                  nodefaultroute usepeerdns persist maxfail 1 \
                  user $APN_USER \
                  password $APN_PASS \
                  connect 'DIALNUMBER='"$dialnumber"' USE_APN='"$APN"' /usr/sbin/chat -t5 -v -E -S -r '"$chatlog"' -f /etc/chatscripts/'"$apnscript"'' \
                  ip-up-script /lib/netifd/ppp-up ipv6-up-script \
                  /lib/netifd/ppp-up ip-down-script /lib/netifd/ppp-down \
                  ipv6-down-script /lib/netifd/ppp-down hide-password \
                  noauth noipdefault noaccomp nopcomp novj nobsdcomp \
                  noauth lock crtscts 115200 $ttydevice 2>&1 >/dev/null &
            pid=$!
        else
            sleep 3 # Wait for chat log file update

            # Update status
            # NOTE: the USBX or MODEMX id is used as PPP network interface!
            ifconfig | grep $ifname 2>/dev/null
            retval=$?
            if [ $retval -eq 0 ]; then
                statusvalue="CONNECTED SUCCESSFULLY"
            else
                grep 'NO CARRIER' $chatlog 2>&1 >/dev/null
                if [ "$?" -eq 0 ]; then
                    statusvalue="NO CARRIER or INVALID INPUT"
                else
                    statusvalue="INVALID INPUT"
                fi
            fi

            # Store status
            uci -c $modem_config_path set modems.$ifname.status="$statusvalue"
            uci -c $modem_config_path commit modems
        fi
    done
}

cleanup() {
    while kill -0 $pid 2>&1 > /dev/null; do # pppd holds for cleanup so we should not exit
        kill -9 $pid
        sleep 1
    done
	kill -9 $sig_strength_pid
	rm -rf $tmpdir/$ifname"_periodic.txt"
    rm -rf $pidfile
    exit 0
}

#
# Main Program Starts here
#    Sample arguments to this script
#    -ifname 3gwan41
#

# Procesing the arguments
while echo $1 | grep ^- > /dev/null; do
    eval $( echo $1 | sed 's/-//g' | tr -d '\012')=$2
    shift
    shift
done

# Global variables
. /etc/modems/modem.path

# Load profile
local PROFILE=${modem_config_path}/${ifname}.profile
if [ -f ${PROFILE} ]; then
    . ${PROFILE}
fi

pidfile="$tmpdir/$ifname.pid"
pid=""
chatlog="$tmpdir/ppp"$ifname".log"

trap cleanup SIGINT SIGHUP SIGTERM
sleep 5 # waiting for old process cleanup

pid="65536" # Just an not running id, to enter the first check
echo $$ > $pidfile
restart_pppd
