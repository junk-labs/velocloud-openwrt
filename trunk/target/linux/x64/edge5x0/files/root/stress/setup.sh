#!/bin/sh

cat > /etc/config/network <<-EOF
config interface 'loopback'
       option ifname 'lo'
       option proto 'static'
       option ipaddr '127.0.0.1'
       option netmask '255.0.0.0'

EOF

# LAN ports:
#   1=(1,1)  3=(1,3)   5=(0,2)  7=(0,1)
#   2=(1,0)  4=(1,2)   6=(0,0)  8=(0,3)
LAN_IFLIST="sw1p1 sw1p0 sw1p3 sw1p2 sw0p2 sw0p0 sw0p1 sw0p3"
GE_IFLIST="eth2 eth3"
SFP_IFLIST="eth4 eth5"
#GE_IFLIST="ge1 ge2"
#SFP_IFLIST="sfp1 sfp2"

IX=0
for IFNAME in $LAN_IFLIST ; do
	IX=$((IX+1))
	cat >> /etc/config/network <<-EOF
config interface 'LAN$IX'
       option ifname '$IFNAME'
       option proto 'static'

EOF
done

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

if grep -q dsa=7 /etc/modules.d/35-igb > /dev/null 2>&1 ; then
    echo "Restarting network to apply new configuration"
    /etc/init.d/network restart
else
    echo "igb dsa=7" > /etc/modules.d/35-igb

    echo "Rebooting to apply new configuration"
    sleep 3
    reboot
fi
