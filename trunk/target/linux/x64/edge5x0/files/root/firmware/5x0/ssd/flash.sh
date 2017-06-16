#!/bin/bash

MYNAME=`readlink $0`
if [ -z "$MYNAME" ]; then
    MYNAME=$0
fi
MYDIR=`dirname "$MYNAME"`
MYDIR=`(cd "$MYDIR" && pwd)`


# /dev/sda:
# 
# ATA device, with non-removable media
#         Model Number:       InnoDisk Corp. - mSATA 3ME4
#         Serial Number:      20170309AA3857000654
#         Firmware Revision:  L17606
#         Media Serial Num:
#         Media Manufacturer:
 
eval $(hdparm -I /dev/sda 2>/dev/null | awk '
/Model Number:/ {
    gsub("^[ \t]*Model Number: *", "")
    gsub(" *$", "")
    printf "DISK_MODEL=\"%s\"\n", $0
}
/Serial Number:/ {
    gsub("^[ \t]*Serial Number: *", "")
    gsub(" *$", "")
    printf "DISK_SERIAL=\"%s\"\n", $0
}
/Firmware Revision:/ {
    gsub("^[ \t]*Firmware Revision: *", "")
    gsub(" *$", "")
    printf "DISK_OLDFWREV=\"%s\"\n", $0
}
')

if [[ $DISK_MODEL =~ InnoDisk.*mSATA ]]; then
    echo "Disk is INNODISK, flashing" > /dev/kmsg
else
    echo "*** INNODISK SSD not found, exiting" > /dev/kmsg
    exit 1
fi

hdparm --yes-i-know-what-i-am-doing --please-destroy-my-drive --fwdownload "$MYDIR"/A_COMBO_L17606.bin /dev/sda > /dev/kmsg 2>&1

#hdparm -i /dev/sda
eval $(hdparm -I /dev/sda 2>/dev/null | awk '
/Firmware Revision:/ {
    gsub("^[ \t]*Firmware Revision: *", "")
    gsub(" *$", "")
    printf "DISK_NEWFWREV=\"%s\"\n", $0
}
')

if [ "$DISK_NEWFWREV" != "L17606" ]; then
    echo "*** FAILED TO UPDATE FIRMWARE *** Old FW rev = $DISK_OLDFWREV, New FW rev = $DISK_NEWFWREV" > /dev/kmsg
    exit 1
else
    echo "Updated Firmware: Old FW rev = $DISK_OLDFWREV, New FW rev = $DISK_NEWFWREV" > /dev/kmsg
    exit 0
fi
