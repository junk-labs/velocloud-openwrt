#!/bin/bash

if [ -z $VELOCLOUD ]; then
    echo "error: need VELOCLOUD envvar set to the IP address or server to access to"
    exit 1
fi

#SSH_PREFS="-o PreferredAuthentications=password"
SSH_PREFS=""

scp ${SSH_PREFS}                                 \
    modem_support/30-usb_ip                      \
    root@${VELOCLOUD}:/etc/hotplug.d/net/

scp ${SSH_PREFS}                                 \
    modem_support/30-config                      \
    root@${VELOCLOUD}:/etc/hotplug.d/tty/

scp ${SSH_PREFS}                                 \
    modem_support/modem_service.sh               \
    modem_support/modem_functions.sh             \
    modem_support/modem.path                     \
    modem_support/usb_common                     \
    modem_support/hybridmodems.list              \
    root@${VELOCLOUD}:/opt/vc/modems/

scp ${SSH_PREFS}                                 \
    modem_support/IPModems/IPModemsRun.py        \
    modem_support/IPModems/IPModems.py           \
    modem_support/IPModems/qmi.py                \
    modem_support/IPModems/sierra.py             \
    modem_support/IPModems/uml290.py             \
    modem_support/IPModems/huawei.py             \
    modem_support/IPModems/qmihybrid.py          \
    root@${VELOCLOUD}:/opt/vc/modems/IPModems/

scp ${SSH_PREFS}                                 \
    modem_support/IPModems/huaweistatus.gcom     \
    modem_support/IPModems/qmihybridstatus.gcom  \
    root@${VELOCLOUD}:/etc/gcom/
