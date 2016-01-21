#!/bin/sh

# Set with some default values if the variables are NULL
set_values_check() {
        apn="`echo "${apn}" | awk '{gsub(/^ +| +$/,"")} {print \$0}'`"
        if [ "$apn" = "" ]; then
                apnscript="3g_noapn.chat"
        else
                apnscript="3g_apn.chat"
        fi
        if [ -z "$username" ]; then
                username="dummy"
        fi
        if [ -z "$password" ]; then
                password="dummy"
        fi
        if [ -z "$dialnumber" ];then
                dialnumber="*99***1#"
        fi
}

# Get the values from the providers DB /etc/modems/TTYModems/isp
getvalues_default_config() {
        local ispindex=$1
	local path=$modem_script_path/TTYModems
        apn=$(uci -c $path get isp.isp$ispindex.apn)
        username=$(uci -c $path get isp.isp$ispindex.username)
        password=$(uci -c $path get isp.isp$ispindex.password)
        dialnumber=$(uci -c $path get isp.isp$ispindex.dialnumber)

        set_values_check
}

# Get the values from the backup configurations
getvalues_backup_config() {
        local ifname=$1
        apn=$(uci -c $db_dir get $ifname.$ifname.apn)
        username=$(uci -c $db_dir get $ifname.$ifname.username)
        password=$(uci -c $db_dir get $ifname.$ifname.password)
        dialnumber=$(uci -c $db_dir get $ifname.$ifname.dialnumber)

        set_values_check
}

# Get the total number of Providers
getvalues_default_providers() {
        echo $(uci -c $modem_script_path/TTYModems show isp | grep -c '=providers')
}

# Get the total number of Backups
getvalues_backup_totalcfgs() {
        echo $(ls -d $modem_path/$wanstring* | wc -l)
}

# Get the values one by one with the interface corresponding as first
# Rest will come as ascending order
getvalues_backup_onebyone() {
        if [ $1 -eq 1 ];then # Get the backup one first then go for next
                echo $(find $db_dir -type f | grep $ifname | xargs basename)
        else
                local omit_backup=$(expr $1 - 1)
                echo $(find $db_dir -type f | grep -v $ifname | head -$omit_backup | tail -1 | xargs basename)
        fi

}

# pppd deamon will be restarted when there is no carrier or when exited due to invalid input
restart_pppd() {
        local apnscript=""
        local status=1
        local index=0
        local STATUSVALUE=""
        local total_default_providers total_backup_config
        local arraylen=0
        local indextmp=0
        local apntmp usernametmp passwordtmp dialnumbertmp
        local retval
        local useroverride


        ttydevice=$(uci -c $modem_path get modems.$ifname.device) # Note only ifname
        useroverride=$(uci -c $modem_path get modems.$ifname.userOverride)

        total_default_providers=$(getvalues_default_providers)
        total_backup_config=$(getvalues_backup_totalcfgs)
        if [ "$useroverride" == "1" ];then
                arraylen=1
        else
                arraylen=$(expr $total_default_providers + $total_backup_config)
        fi

        index=1 # start from backup files as per ascending order(Except the current interface)

	nohup $modem_script_path/TTYModems/get_sig_strength.sh $ifname 2>&1 >/dev/null &
	sig_strength_pid=$!
        # if file is removed then dont restart the pppd
        while [ -f $pidfile ]; do
                if [ $index -le $total_backup_config ];then
                        getvalues_backup_config "$(getvalues_backup_onebyone $index)"
                else
                        indextmp=$(expr $total_default_providers - $arraylen + $index)
                        getvalues_default_config $indextmp
                fi
                if ! kill -0 $pid 2>&1 > /dev/null;then
                        > $chatlog # Empty the log
                        apntmp="$apn"
                        usernametmp="$username"
                        passwordtmp="$password"
                        dialnumbertmp="$dialnumber"

                        nohup /usr/sbin/pppd nodetach ipparam wan ifname $ifname \
                        nodefaultroute usepeerdns persist maxfail 1 \
                        user $username \
                        password $password \
                        connect 'DIALNUMBER='"$dialnumber"' USE_APN='"$apn"' /usr/sbin/chat -t5 -v -E -S -r '"$chatlog"' -f /etc/chatscripts/'"$apnscript"'' \
                        ip-up-script /lib/netifd/ppp-up ipv6-up-script \
                        /lib/netifd/ppp-up ip-down-script /lib/netifd/ppp-down \
                        ipv6-down-script /lib/netifd/ppp-down hide-password \
                        noauth noipdefault noaccomp nopcomp novj nobsdcomp \
                        noauth lock crtscts 115200 $ttydevice 2>&1 >/dev/null &
                        pid=$!
                        if [ "$index" -lt $arraylen ]; then
                                index=$(expr $index + 1)
                        else
                                index=1
                        fi
                else
                        sleep 3 # Wait for chat log file update

                        ifconfig | grep $ifname 2>/dev/null
                        retval=$?
                        if [ $retval -eq 0 ]; then
                                uci -c $modem_path set modems.$ifname.apn=''"$apntmp"''
                                uci -c $modem_path set modems.$ifname.username=''"$usernametmp"''
                                uci -c $modem_path set modems.$ifname.password=''"$passwordtmp"''
                                uci -c $modem_path set modems.$ifname.dialnumber=''"$dialnumbertmp"''
                                STATUSVALUE="CONNECTED SUCCESSFULLY"
                                index=1         # Reset to 1 so when disconnected let it try from backup
                                status=0        # To stop entering to 'NO CARRIER'
                        else
                                status=1        # To enter in to 'NO CARRIER'
                        fi

                        if [ "$status" -eq 1 ]; then
                                grep 'NO CARRIER' $chatlog 2>&1 >/dev/null
                                if [ "$?" -eq 0 ]; then
                                        STATUSVALUE="NO CARRIER or INVALID INPUT"
                                else
                                        STATUSVALUE="INVALID INPUT"
                                fi
                        fi

                        uci -c $modem_path set modems.$ifname.status="$STATUSVALUE"
                        uci -c $modem_path commit modems
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

cleanup_oldprocess() {
        sleep 5 # waiting for old process cleanup
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
apn=""
username=""
password=""
dialnumber=""
ttydevice=""

pidfile="$tmpdir/$ifname.pid"
pid=""
chatlog="$tmpdir/ppp"$ifname".log"

trap cleanup SIGINT SIGHUP SIGTERM
cleanup_oldprocess

pid="65536" # Just an not running id, to enter the first check
echo $$ > $pidfile
restart_pppd

