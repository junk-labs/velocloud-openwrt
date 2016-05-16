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

# config:
#
IPADDR_GE1=10.0.0.11
IPADDR_GE2=10.0.0.12
IPADDR_GE3=10.0.0.13
IPADDR_GE4=10.0.0.14
IPADDR_GE5=10.0.0.15
IPADDR_GE6=10.0.0.16
IPADDR_GE7=10.0.0.17
IPADDR_GE8=10.0.0.18
#
IPADDR_SFP1=10.0.0.21
IPADDR_SFP2=10.0.0.22
#
# Wiring:
#   SFP1 to SFP2
#   GE1 to GE5
#   GE2 to GE6
#   GE3 to GE7
#   GE4 to GE8
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
# $1 and $2 are the physical interface names;
# $3 and $4 are their respective IPs;
# $5 and $6 are their respective table IDs;

setup_loopback_physical() {
        echo "Setting up loop $1/$2"
        echo 1 > /proc/sys/net/ipv4/conf/$1/accept_local
        echo 1 > /proc/sys/net/ipv4/conf/$2/accept_local
	ifconfig $1 $3
	ifconfig $2 $4
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

# Configures the interface and sets up routing between two interfaces
# $1 and $2 are symbolic interface names (GE1, SFP2, etc.)
TABLE_NUM=1
setup_loop() {
	LIF1="$1"
	LIF2="$2"
	PHYS1=`uci get network.$LIF1.ifname`
	PHYS2=`uci get network.$LIF2.ifname`

	# configure IP addresses
	IP1=`eval echo '$'IPADDR_$LIF1`
	IP2=`eval echo '$'IPADDR_$LIF2`

	TN1=$((TABLE_NUM+1))
	TN2=$((TABLE_NUM+2))
	TABLE_NUM=$TN2
	setup_loopback_physical $PHYS1 $PHYS2 $IP1 $IP2 $TN1 $TN2
}

trap clean_up 2
clean_up

# setup loopback;

echo "Setting up loopback"

setup_loop GE1 GE5
setup_loop GE2 GE6
setup_loop GE3 GE7
setup_loop GE4 GE8
setup_loop SFP1 SFP2

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
iperf -s -p 5013 >/dev/null &
iperf -s -p 5014 >/dev/null &
iperf -s -p 5015 >/dev/null &
iperf -s -p 5016 >/dev/null &
iperf -s -p 5017 >/dev/null &
iperf -s -p 5018 >/dev/null &

iperf -s -p 5021 >/dev/null &
iperf -s -p 5022 >/dev/null &
sleep 2
(
iperf -p 5011 -c 10.0.0.11 $iperf_int -t $dur > /tmp/iperf.5011.out 2>&1 &
iperf -p 5012 -c 10.0.0.12 $iperf_int -t $dur > /tmp/iperf.5012.out 2>&1 &
iperf -p 5013 -c 10.0.0.13 $iperf_int -t $dur > /tmp/iperf.5013.out 2>&1 &
iperf -p 5014 -c 10.0.0.14 $iperf_int -t $dur > /tmp/iperf.5014.out 2>&1 &
iperf -p 5015 -c 10.0.0.15 $iperf_int -t $dur > /tmp/iperf.5015.out 2>&1 &
iperf -p 5016 -c 10.0.0.16 $iperf_int -t $dur > /tmp/iperf.5016.out 2>&1 &
iperf -p 5017 -c 10.0.0.17 $iperf_int -t $dur > /tmp/iperf.5017.out 2>&1 &
iperf -p 5018 -c 10.0.0.18 $iperf_int -t $dur > /tmp/iperf.5018.out 2>&1 &

iperf -p 5021 -c 10.0.0.21 $iperf_int -t $dur > /tmp/iperf.5021.out 2>&1 &
iperf -p 5022 -c 10.0.0.22 $iperf_int -t $dur > /tmp/iperf.5022.out 2>&1 &
wait
rm -f $SENTINEL
) &

LINES=24
if stty -g -F /proc/self/fd/0 > /dev/null 2>&1; then
    # stdin is a tty
    eval `resize`
    LINES=`stty size | cut -d' ' -f1`
fi
NUM_IPERFS=`pidof iperf | wc -w`
TOPLINES=`expr $LINES - $NUM_IPERFS / 2 - 3`

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
    #IPERF_PIDS=`pidof iperf openssl | sed -e 's/ /,/g'`
    #if [ -z "$IPERF_PIDS" ]; then
    #    IPERF_PIDS=99999999
    #fi
    #top -b -c -n 1 -p $IPERF_PIDS | head -$TOPLINES
    top -b -c -n 1 | head -$TOPLINES
}

while test -e $SENTINEL ; do
    report_progress
    killall -0 iperf > /dev/null 2>&1 || break
    sleep 5
done

clean_up
report_progress
