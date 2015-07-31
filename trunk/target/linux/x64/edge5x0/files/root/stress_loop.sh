#!/bin/ash
# local loopback stress;

iperf_int="-i 10"
dur=360000
#dur=60
run_openssl=0

while getopts sd: opt; do
    case $opt in
        d) dur=$OPTARG ;;
        s) run_openssl=1 ;;
        \?) exit 1 ;;
    esac
done

iperfs=""
openssls=""

# LAN ports:
#   1=(1,1)  3=(1,3)   5=(0,2)  7=(0,1)
#   2=(1,0)  4=(1,2)   6=(0,0)  8=(0,3)
#

# config:
#
#   LAN1 (sw1p1):  10.0.0.10
#   LAN2 (sw1p0):  10.0.0.11
#   LAN3 (sw1p3):  10.0.0.12
#   LAN4 (sw1p2):  10.0.0.13
#
#   LAN5 (sw0p2):  10.0.0.20
#   LAN6 (sw0p0):  10.0.0.21
#   LAN7 (sw0p1):  10.0.0.22
#   LAN8 (sw0p3):  10.0.0.23
#
#   WAN1 (eth2): 10.0.0.32
#   WAN2 (eth3): 10.0.0.33
#   SFP1 (eth4): 10.0.0.34
#   SFP2 (eth5): 10.0.0.35
#
# Wiring:
#   SFP1 to LAN1
#   SFP2 to LAN4
#   WAN1 to LAN5
#   WAN2 to LAN8
#   LAN2 to LAN6
#   LAN3 to LAN7
#

export SENTINEL=/tmp/stress.$$

clean_up() {
        openssls=$(pgrep openssl)
        if [ -n "$openssls" ]; then
                kill -9 $openssls
        fi

        iperfs=$(pgrep iperf)
        if [ -n "$iperfs" ]; then
                kill -9 $iperfs
        fi

	rm -f $SENTINEL
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

ifconfig sw1p1 10.0.0.10
ifconfig sw1p0 10.0.0.11
ifconfig sw1p3 10.0.0.12
ifconfig sw1p2 10.0.0.13

ifconfig sw0p2 10.0.0.20
ifconfig sw0p0 10.0.0.21
ifconfig sw0p1 10.0.0.22
ifconfig sw0p3 10.0.0.23

ifconfig eth2 10.0.0.32
ifconfig eth3 10.0.0.33

ifconfig eth4 10.0.0.34
ifconfig eth5 10.0.0.35

# setup loops;

setup_loopback eth4 sw1p1 10.0.0.34 10.0.0.10 2 3
setup_loopback eth5 sw1p2 10.0.0.35 10.0.0.13 4 5
setup_loopback eth2 sw0p2 10.0.0.32 10.0.0.20 6 7
setup_loopback eth3 sw0p3 10.0.0.33 10.0.0.23 8 9

setup_loopback sw1p0 sw0p0 10.0.0.11 10.0.0.21 10 11
setup_loopback sw1p3 sw0p1 10.0.0.12 10.0.0.22 12 13

# exit 0

touch $SENTINEL

# start openssl stress;
if [ "$run_openssl" != "0" ]; then
    echo "Starting openssl stress"
    (
    while test -r $SENTINEL ; do
        openssl speed 2>/dev/null >/tmp/x1 &
        openssl speed 2>/dev/null >/tmp/x2 &
        wait
    done
    ) &
fi

# kill old iperfs laying around;
# start iperfs;

echo "Starting iperf on loopbacks"
iperf -s -p 5011 >/dev/null &
iperf -s -p 5012 >/dev/null &
iperf -s -p 5032 >/dev/null &
iperf -s -p 5033 >/dev/null &
iperf -s -p 5034 >/dev/null &
iperf -s -p 5035 >/dev/null &
sleep 2
(
iperf -p 5011 -c 10.0.0.11 $iperf_int -t $dur > /tmp/iperf.5011.out 2>&1 &
iperf -p 5012 -c 10.0.0.12 $iperf_int -t $dur > /tmp/iperf.5012.out 2>&1 &
iperf -p 5032 -c 10.0.0.32 $iperf_int -t $dur > /tmp/iperf.5032.out 2>&1 &
iperf -p 5033 -c 10.0.0.33 $iperf_int -t $dur > /tmp/iperf.5033.out 2>&1 &
iperf -p 5034 -c 10.0.0.34 $iperf_int -t $dur > /tmp/iperf.5034.out 2>&1 &
iperf -p 5035 -c 10.0.0.35 $iperf_int -t $dur > /tmp/iperf.5035.out 2>&1 &
wait
rm -f $SENTINEL
) &

LINES=24
if stty -g -F /proc/self/fd/0 > /dev/null 2>&1; then
    # stdin is a tty
    eval `resize`
    LINES=`stty size | cut -d' ' -f1`
fi
TOPLINES=`expr $LINES - 9`

# Run top until all iperfs finish
# top

report_progress()
{
    clear
    echo "IPERF runs:"
    for F in /tmp/iperf.*.out; do
        FN=`basename $F .out`
	FSTATUS=`tail +6 $F | tail -1`
	echo "    $FN: $FSTATUS"
    done
    echo "Processes:"
    IPERF_PIDS=`pidof iperf openssl | sed -e 's/ /,/g'`
    if [ -z "$IPERF_PIDS" ]; then
        IPERF_PIDS=99999999
    fi
    top -b -c -n 1 -p $IPERF_PIDS | head -$TOPLINES
}

while test -e $SENTINEL ; do
    report_progress
    killall -0 iperf > /dev/null 2>&1 || break
    sleep 5
done

clean_up
report_progress
