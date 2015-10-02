#!/bin/sh

cleanup_old() {
        local pid=`cat $pidfile`
        while kill -0 $pid 2>&1 > /dev/null; do
                kill -9 $pid
                sleep 1
        done
        rm -rf $tmpdir/$ifname"_periodic.txt"
        rm -rf $pidfile
}
cleanup_self() {
        rm -rf $tmpdir/$ifname"_periodic.txt"
        rm -rf $pidfile
        exit 0
}

log_modem_stats() {
        device=$1
        intf=$2
        value=$(gcom -s /etc/gcom/getstrength.gcom -d $device | grep '+CSQ:' | awk -F":|," '{print $2}')

        # Marginal - Levels of -95dBm or lower
        # Workable under most conditions - Levels of -85dBm to -95dBm
        # Good - Levels between -75dBm and -85dBm
        # Excellent - levels above -75dBm.

        strength=$(expr 113 - $value - $value)
        strength=-$strength
        sigpercentage=0
        if [ $strength -le -95 ]; then sigpercentage=25;
        elif [[ $strength -le -95 ]] && [[ $strength -ge -85 ]]; then sigpercentage=50;
        elif [[ $strength -le -85 ]] && [[ $strength -ge -75 ]]; then sigpercentage=75;
        elif [ $strength -ge -75 ]; then sigpercentage=100;
        fi

        # TODO: It might be good to add some code to monitor the Rx packets/bytes from the 
        # intf and reset the modem if the Rx has been stalled for long (like say 30 seconds)
        # Currently the modem seems to go into a wierd state when no packet activity and that
        # "wierdness" is observed by the atlockstatus.gcom returning value 0. In such a case,
        # we already take action and reset the modem. Maybe thats sufficient ? Also note that
        # this modem going into a "wierd state" / "hung" (ie no further Rx/Tx) happens even 
        # on a Mac laptop with VZAccessManager managing the modem
        rxbytes=$(ifconfig $intf | awk  '/RX packets:/ { sub(/packets:/, "");print $2}')
        txbytes=$(ifconfig $intf | awk  '/TX packets:/ { sub(/packets:/, "");print $2}')

        echo -n '{ "SigStrength" : '"$strength"', "SigPercentage" : '"$sigpercentage"', "Rxbytes" : '"$rxbytes"', "Txbytes" : '"$txbytes"' }' > "$tmpdir/""$ifname"_periodic.txt
}

start_modem_monitor() {

        local mgmt=$(uci_get_modem_mgmt $ifname)
        local intf=$(uci_get_modem_ifname $ifname)

        if [ ! -n "$mgmt" ]; then
                log "USB Modem: $ifname, cant find mgmt dev, exiting"
                exit 0
        fi
        if [ ! -n "$intf" ]; then
                log "USB Modem: $ifname, cant find data intf, exiting"
                exit 0
        fi

        log "USB Modem: restart qmi $ifname mgmt $mgmt data $intf"
 
        while true;
        do
                # first time init
                if [ "$qmi_status" == "0" ]; then
                            count=1
                            while [ $count -le 10 ];
                            do
                                    log "USB Modem: connecting $ifname, attempt: $count"
                                    gcom -d $mgmt -s /etc/gcom/qmiconnect.gcom 
                                    sleep 5
                                    qmistate=`gcom -d $mgmt -s /etc/gcom/qmistatus.gcom | grep "QMI State: CONNECTED"`
                                    wdsstate=`gcom -d $mgmt -s /etc/gcom/qmistatus.gcom | grep "QMI State: QMI_WDS_PKT_DATA_CONNECTED"`
                                    if [ ! -z "$qmistate" ] || [ ! -z "$wdsstate" ]; then
                                            break
                                    fi    
                                    count=$(($count+1))
                            done    
                            # Either we connected or we dint, if we are not connected, then the else
                            # clause will bring us back here after a reset
                            qmi_status=1
                else
                        connstatus=`gcom -d $mgmt -s /etc/gcom/qmistatus.gcom | grep DISCONNECTED`
                        if [ ! -z "$connstatus" ]; then
                            # Modem was working, now it seems to be disconnected, I have seen 
                            # at times that when modem goes from connected to disconnected state
                            # after being connected for long, it doesnt help trying an NWQMICONNECT
                            # again, it really needs a knock on the head and then a NWQMICONNECT
                            log "USB Modem: $ifname back to disconnected mode, trying a reset"
                            gcom -d $mgmt -s /etc/gcom/atreset.gcom
                            # After a reset we get a hotplug remove event, so there is no question
                            # of "going back" or anything here, but just code assume we will go back
                            # to the beginning of the while loop
                            qmi_status=0
                            sleep 10
                            continue
                        fi
                fi

                log_modem_stats $mgmt $intf
                sleep 15
        done
}

# Procesing the arguments
while echo $1 | grep ^- > /dev/null; do
        eval $( echo $1 | sed 's/-//g' | tr -d '\012')=$2
        shift
        shift
done

# Global variables
. /opt/vc/modems/modem.path
. /opt/vc/modems/usb_common
pidfile="$tmpdir/$ifname.pid"
if [ -f $pidfile ]; then
    cleanup_old
fi
trap cleanup_self SIGINT SIGHUP SIGTERM
echo $$ > $pidfile

qmi_status=0
start_modem_monitor

