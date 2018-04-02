#!/bin/bash

if [ ! -d ./files/etc/modems ]; then
    echo "error: the script must be run from the top-level 'modems' package directory"
    exit 1
fi

if [ -z "${VELOCLOUD}" ]; then
    echo "error: need VELOCLOUD envvar set to the IP address or server to access to"
    exit 1
fi

SCP_CMD=$(which scp)

if [ -z "${SCP_CMD}" ]; then
    echo "error: need 'scp' program installed"
    exit 1
fi

if [ -n "${VELOCLOUD_PASSWORD}" ]; then
    SSHPASS=$(which sshpass)
    if [ -z "${SSHPASS}" ]; then
        echo "error: need 'sshpass' program installed"
        exit 1
    fi

    SCP_CMD="${SSHPASS} -p ${VELOCLOUD_PASSWORD} scp -o PreferredAuthentications=password"
fi

${SCP_CMD}                                           \
    ./files/etc/hotplug.d/net/30-velocloud-modem-net \
    root@${VELOCLOUD}:/etc/hotplug.d/net/

${SCP_CMD}                                           \
    ./files/etc/hotplug.d/tty/30-velocloud-modem-tty \
    root@${VELOCLOUD}:/etc/hotplug.d/tty/

${SCP_CMD}                                           \
    ./files/etc/modems/modem_service.sh              \
    ./files/etc/modems/modem_apn.sh                  \
    ./files/etc/modems/modem.path                    \
    ./files/etc/modems/usb_common                    \
    ./files/etc/modems/hybridmodems.list             \
    ./files/etc/modems/get_avg_sig_strength.py       \
    root@${VELOCLOUD}:/etc/modems/

${SCP_CMD}                                           \
    ./files/etc/modems/IPModems/IPModemsRun.py       \
    ./files/etc/modems/IPModems/IPModems.py          \
    ./files/etc/modems/IPModems/qmi.py               \
    ./files/etc/modems/IPModems/pantech.py           \
    ./files/etc/modems/IPModems/huawei.py            \
    ./files/etc/modems/IPModems/ubee.py              \
    ./files/etc/modems/IPModems/zte.py               \
    ./files/etc/modems/IPModems/sierranet.py         \
    ./files/etc/modems/IPModems/qmihybrid.py         \
    root@${VELOCLOUD}:/etc/modems/IPModems/

${SCP_CMD}                                           \
    ./files/etc/modems/TTYModems/ppp_run.sh          \
    root@${VELOCLOUD}:/etc/modems/TTYModems/

${SCP_CMD}                                           \
    ./files/etc/gcom/getcaps.gcom                    \
    ./files/etc/gcom/huaweistatus.gcom               \
    ./files/etc/gcom/qmihybridstatus.gcom            \
    ./files/etc/gcom/sierranetstatus.gcom            \
    root@${VELOCLOUD}:/etc/gcom/
