#!/bin/sh

MYNAME=`readlink -f $0`
MYDIR=`dirname "$MYNAME"`
MYDIR=`(cd "$MYDIR" && pwd)`

BOARD=`cat /sys/devices/platform/vc/board`

cat > /etc/config/network <<EOF
config interface 'loopback'
	option ifname 'lo'
	option proto 'static'
	option ipaddr '127.0.0.1'
	option netmask '255.0.0.0'
	option ipv6 '0'

config interface 'lan'
	option ifname ''
	option proto 'static'
	option type 'bridge'
	option ipaddr '192.168.2.1'
	option netmask '255.255.255.0'
	option ipv6 '0'

EOF

if [ "$BOARD" = "edge510" ]; then
	LAN_IFLIST=""
	GE_IFLIST="eth0 eth1 eth2 eth3"
	SFP_IFLIST=""
else
	# LAN ports:
	#   1=(1,1)  3=(1,3)   5=(0,2)  7=(0,1)
	#   2=(1,0)  4=(1,2)   6=(0,0)  8=(0,3)
	LAN_IFLIST="sw1p1 sw1p0 sw1p3 sw1p2 sw0p2 sw0p0 sw0p1 sw0p3"
	GE_IFLIST="eth2 eth3"
	SFP_IFLIST="eth4 eth5"
	#GE_IFLIST="ge1 ge2"
	#SFP_IFLIST="sfp1 sfp2"
fi

IX=0
for IFNAME in $LAN_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<EOF
config interface 'LAN$IX'
	option ifname '$IFNAME'
	option proto 'static'
	option ipv6 '0'

EOF
done

IX=0
for IFNAME in $GE_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<EOF
config interface 'GE$IX'
	option ifname '$IFNAME'
	option proto 'static'
	option ipv6 '0'

EOF
done

IX=0
for IFNAME in $SFP_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<EOF
config interface 'SFP$IX'
	option ifname '$IFNAME'
	option proto 'static'
	option ipv6 '0'

EOF
done

echo "Enabling password-based root sshd login"
sed -i -e 's/^#\?PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config

if [ "$BOARD" = "edge510" ]; then
    :
else
    # Copy a disabled wifi configuration by default
    echo "Disabling wi-fi by default. See /root/stress/wifi-cfg/README.wireless.txt"
    cp $MYDIR/wifi-cfg/wireless.emissions /etc/config/wireless
fi

if [ "$BOARD" = "edge510" ]; then
    echo "Restarting network to apply new configuration"
    /etc/init.d/network restart
else
    if grep -q dsa=7 /etc/modules.d/35-igb > /dev/null 2>&1 ; then
        echo "Restarting network to apply new configuration"
        /etc/init.d/network restart
    else
        echo "igb dsa=7" > /etc/modules.d/35-igb

        echo "Rebooting to apply new configuration"
        sleep 3
        reboot
    fi
fi
