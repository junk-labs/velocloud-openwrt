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
IPADDR_LAN1=10.0.0.11   ; PORT_LAN1=5011
IPADDR_LAN2=10.0.0.12	; PORT_LAN2=5012
IPADDR_LAN3=10.0.0.13	; PORT_LAN3=5013
IPADDR_LAN4=10.0.0.14   ; PORT_LAN4=5014
IPADDR_LAN5=10.0.0.15   ; PORT_LAN5=5015
IPADDR_LAN6=10.0.0.16   ; PORT_LAN6=5016
IPADDR_LAN7=10.0.0.17   ; PORT_LAN7=5017
IPADDR_LAN8=10.0.0.18   ; PORT_LAN8=5018
#
IPADDR_GE1=10.0.0.21	; PORT_GE1=5021
IPADDR_GE2=10.0.0.22	; PORT_GE2=5022
#
IPADDR_SFP1=10.0.0.31	; PORT_SFP1=5031
IPADDR_SFP2=10.0.0.32	; PORT_SFP2=5032
#
# Wiring:
#   SFP1 to SFP2
#   GE1 to GE2
#   LAN1 to LAN5
#   LAN2 to LAN6
#   LAN3 to LAN7
#   LAN4 to LAN8

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

# From the Wiring above

setup_loop SFP1 SFP2
setup_loop GE1  GE2
setup_loop LAN1 LAN5
setup_loop LAN2 LAN6
setup_loop LAN3 LAN7
setup_loop LAN4 LAN8

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

rm -f /tmp/iperf.*.out

echo "Starting iperf on loopbacks"
iperf -s -p $PORT_LAN1 >/dev/null &
iperf -s -p $PORT_LAN2 >/dev/null &
iperf -s -p $PORT_LAN7 >/dev/null &
iperf -s -p $PORT_LAN8 >/dev/null &
iperf -s -p $PORT_GE1 >/dev/null &
iperf -s -p $PORT_SFP1 >/dev/null &
sleep 2
(
iperf -p $PORT_LAN1 -c $IPADDR_LAN1 $iperf_int -t $dur > /tmp/iperf.$PORT_LAN1.out 2>&1 &
iperf -p $PORT_LAN2 -c $IPADDR_LAN2 $iperf_int -t $dur > /tmp/iperf.$PORT_LAN2.out 2>&1 &
iperf -p $PORT_LAN7 -c $IPADDR_LAN7 $iperf_int -t $dur > /tmp/iperf.$PORT_LAN7.out 2>&1 &
iperf -p $PORT_LAN8 -c $IPADDR_LAN8 $iperf_int -t $dur > /tmp/iperf.$PORT_LAN8.out 2>&1 &
iperf -p $PORT_GE1 -c $IPADDR_GE1 $iperf_int -t $dur > /tmp/iperf.$PORT_GE1.out 2>&1 &
iperf -p $PORT_SFP1 -c $IPADDR_SFP1 $iperf_int -t $dur > /tmp/iperf.$PORT_SFP1.out 2>&1 &

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
