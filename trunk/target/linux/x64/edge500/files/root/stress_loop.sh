#!/bin/ash
# local loopback stress;

iperf_int="-i 10"
dur=360000
#dur=3600

iperfs=""
openssls=""

# Edge300 config:
#
#   GE1 (eth0):  10.0.0.10
#   GE2 (eth1):  10.0.0.11
#   GE3 (eth2):  10.0.0.12
#   GE4 (eth3):  10.0.0.13
#
# Wiring:
#   GE1 to GE2
#   GE3 to GE4
#


clean_up() {
        openssls=$(pgrep openssl)
        if [ -n "$openssls" ]; then
                kill -9 $openssls
        fi

        iperfs=$(pgrep iperf)
        if [ -n "$iperfs" ]; then
                kill -9 $iperfs
        fi
}

# setup a loopback pair;
# $1 and $2 are the interface names;
# $3 and $4 are their respective IPs;
# $5 and $6 are their respective table IDs;

setup_loopback() {
        echo "Setting up loop $1/$2"
        echo 1 > /proc/sys/net/ipv4/conf/$1/accept_local
        echo 1 > /proc/sys/net/ipv4/conf/$2/accept_local
        route del $3
        route del $4
        route add $4 dev $1
        route add $3 dev $2
        ip route del $3 table local 2>/dev/null
        ip route del $4 table local 2>/dev/null
        ip route del $3 table $5
        ip route del $4 table $6
        ip rule add iif $1 lookup $5
        ip route add local $3 dev $1 table $5
        ip rule add iif $2 lookup $6
        ip route add local $4 dev $2 table $6
}

trap clean_up 2
clean_up

# shut down default network config

#echo "Shutting down default network config"

#ifconfig br-lan down
#brctl delif br-lan sw0p0
#brctl delif br-lan sw0p1
#brctl delif br-lan sw0p2
#brctl delif br-lan sw0p3
#brctl delif br-lan sw1p0
#brctl delif br-lan sw1p1
#brctl delif br-lan sw1p2
#brctl delif br-lan sw1p3
#ifconfig br-lan up

# setup loopback;

echo "Setting up loopback"

ifconfig eth0 10.0.0.10
ifconfig eth1 10.0.0.11
ifconfig eth2 10.0.0.12
ifconfig eth3 10.0.0.13

# setup loops;

setup_loopback eth0 eth1 10.0.0.10 10.0.0.11 2 3
setup_loopback eth2 eth3 10.0.0.12 10.0.0.13 4 5

# exit 0

run_openssl()
{
    while nice openssl speed; do
        : nothing
    done
}

# start openssl stress;
# echo "Starting openssl stress"
run_openssl 2>/dev/null >/tmp/x1 &
run_openssl 2>/dev/null >/tmp/x2 &

# kill old iperfs laying around;
# start iperfs;

echo "Starting iperf on loopbacks"
iperf -s -p 5010 >/dev/null &
iperf -s -p 5012 >/dev/null &
sleep 2
(
iperf -p 5010 -c 10.0.0.10 $iperf_int -t $dur > /tmp/iperf.5010.out 2>&1 &
iperf -p 5012 -c 10.0.0.12 $iperf_int -t $dur > /tmp/iperf.5012.out 2>&1 &
wait
killall top
) &

if stty -g -F /proc/self/fd/0 > /dev/null 2>&1; then
    # stdin is a tty
    eval `resize`
fi
# Run top until all iperfs finish
top

clean_up
