#!/bin/sh

cat > /etc/config/network <<-EOF
config interface 'loopback'
       option ifname 'lo'
       option proto 'static'
       option ipaddr '127.0.0.1'
       option netmask '255.0.0.0'

config interface 'lan'
        option ifname ''
        option proto 'static'
        option type 'bridge'
        option ipaddr '192.168.2.1'
        option netmask '255.255.255.0'
EOF

for IFNAME in sw0p0 sw0p1 sw0p2 sw0p3 sw1p0 sw1p1 sw1p2 sw1p3 eth2 eth3 eth4 eth5 ; do
	cat >> /etc/config/network <<-EOF
config interface '$IFNAME'
       option ifname '$IFNAME'
       option proto 'static'

EOF
done

/etc/init.d/network reload

echo "igb dsa=7" > /etc/modules.d/35-igb

echo "Rebooting to apply new configuration"
sleep 3
reboot
