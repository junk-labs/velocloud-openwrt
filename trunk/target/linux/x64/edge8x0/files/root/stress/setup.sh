#!/bin/sh

cat > /etc/config/network <<-EOF
config interface 'loopback'
       option ifname 'lo'
       option proto 'static'
       option ipaddr '127.0.0.1'
       option netmask '255.0.0.0'

EOF

BOARD_NAME=`cat /sys/class/dmi/id/board_name`
if [ "$BOARD_NAME" = "X10SRW-F" ]; then
    # supermicro
    GE_IFLIST="eth6 eth7 eth0 eth1 eth5 eth4 eth3 eth2"
    SFP_IFLIST="eth9 eth8"
else # if [ "$BOARD_NAME" = "S1600JP" ]; then
    # interface masters
    GE_IFLIST="eth8 eth7 eth6 eth4 eth11 eth10 eth9 eth5"
    SFP_IFLIST="eth3 eth2"
fi

IX=0
for IFNAME in $GE_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<-EOF
config interface 'GE$IX'
       option ifname '$IFNAME'
       option proto 'static'

EOF
done

IX=0
for IFNAME in $SFP_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<-EOF
config interface 'SFP$IX'
       option ifname '$IFNAME'
       option proto 'static'

EOF
done

echo "Restarting network to apply new configuration"
/etc/init.d/network restart

# echo "Rebooting to apply new configuration"
# sleep 3
# reboot
