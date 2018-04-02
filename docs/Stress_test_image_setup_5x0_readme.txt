In order to run the stress_loop.sh script, the image has to be set up
in a very particular way, especially module parameters and network configs.

This file describes the steps.  First do a build of "edge5x0", and then
copy the -installer-ext4.img file to a different place to perform the
following steps to fix it up (before flashing it to USB):

# sudo kpartx -av openwrt-xx-installer-xx.img
Let's say this gets mounted as /dev/loopNp0 .. /dev/loopNp3

1. Change the boot default to 4, and selection timeout to 5 seconds:

# mkdir /tmp/boot
# mount /dev/mapper/loopNp1 /tmp/boot
 - Change the 'set default=0' to 'set default=4'
 - Change the 'set timeout=-1' to 'set timeout=5'
# umount /tmp/boot

2. Changes for stress test environment:
   - Change the boot params for the "IGB" module;
   - copy a stress-specific /etc/config/network file;
   - (Temporary) copy in custom igb.ko from build1-losaltos:/tmp/sandra/igb.ko

# mkdir /tmp/root
# mount /dev/mapper/loopNp2 /tmp/root
# echo 'igb dsa=7' > /tmp/root/etc/modules.d/35-igb
# cp /tmp/root/root/network-config.stress /tmp/root/etc/config/network
# cp <custom-igb.ko> /tmp/root/lib/modules/3.14.37/igb.ko
# umount /tmp/boot

PS. Now you can safely delete the loop mounts

# dmsetup remove loopNp0
# dmsetup remove loopNp1
# dmsetup remove loopNp2
# dmsetup remove loopNp3
OR
# dmsetup remove_all   (if these are the only loop mounts - see "dmsetup ls")
