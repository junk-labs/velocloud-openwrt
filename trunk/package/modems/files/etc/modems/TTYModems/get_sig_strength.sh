#!/bin/sh

. /etc/modems/modem.path
USB=$1

while [[ 1 ]]; do
        device=$(uci -c $modem_path get modems.$USB.mgmt)
	if [ ! -n "$device" ]; then
		continue
	fi
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

        rxbytes=$(ifconfig $USB | awk  '/RX packets:/ { sub(/packets:/, "");print $2}')
        txbytes=$(ifconfig $USB | awk  '/TX packets:/ { sub(/packets:/, "");print $2}')

        echo -n '{ "SigStrength" : '"$strength"', "SigPercentage" : '"$sigpercentage"', "Rxbytes" : '"$rxbytes"', "Txbytes" : '"$txbytes"' }' > "$tmpdir/""$USB"_periodic.txt
        sleep 3
done

