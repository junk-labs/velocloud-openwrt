#!/usr/bin/env bash
# Copyright (C) 2013 Veloccloud.net
set -x

[ $# -eq 6 ] || {
    echo "SYNTAX: $0 <file> <boot size> <boot dir> <install root size> <install dirs> <vdisk size>"
    exit 1
}

ARGV=("$@")
OUTPUT="$1"
BOOT_SIZE="$2"
BOOT_DIR="$3"
INST_ROOT_SIZE="$4"
INST_DIR="$5"
VDISK_SIZE="$6"

# translate install images dirs;
# first inst_dir entry must point to a dir without ':' separator;

IROOT="${OUTPUT}.iroot"

root_dirs="${IROOT}"
inst_dirs="-d $INST_DIR:/"

rm -f "$OUTPUT"
rm -rf $IROOT

# align all partition to 1MB boundaries;
# root and installer filesystem are same size;
# INST_ROOT_SIZE is a parameter to the installer;

GRUB_OFF=1
GRUB_END=2

BOOT_OFF=$GRUB_END
BOOT_END="$(($BOOT_OFF + $BOOT_SIZE))"

ROOT0_OFF=$BOOT_END
ROOT0_END="$(($ROOT0_OFF + $INST_ROOT_SIZE))"

ROOT1_OFF=$ROOT0_END
ROOT1_END="$(($ROOT1_OFF + $INST_ROOT_SIZE))"

ROOT2_OFF=$ROOT1_END
ROOT2_END="$(($ROOT2_OFF + $INST_ROOT_SIZE))"

VELOCLOUD_OFF=$ROOT2_END
VELOCLOUD_END="$(($VDISK_SIZE - 2))"

TAIL_OFF=$VELOCLOUD_END
TAIL_SIZE=1
TAIL_END="$(($VDISK_SIZE - 1))"

# copy rootfs image;

#dd if="$ROOT_FSIMAGE" of="$OUTPUT" bs=1M seek="$ROOT_OFF" conv=notrunc count="$ROOT_SIZE" || exit 1
dd if=/dev/zero of="$OUTPUT" bs=1M seek="$TAIL_OFF" conv=notrunc count="$TAIL_SIZE" || exit 1

# create the partition table
parted -s -- $OUTPUT \
	unit MiB \
	mklabel gpt \
	mkpart grub $GRUB_OFF $GRUB_END \
	set 1 bios_grub on \
	mkpart boot $BOOT_OFF $BOOT_END \
	set 2 boot on \
	mkpart root0 $ROOT0_OFF $ROOT0_END \
	mkpart root1 $ROOT1_OFF $ROOT1_END \
	mkpart root2 $ROOT2_OFF $ROOT2_END \
	mkpart user $VELOCLOUD_OFF $VELOCLOUD_END \
	print \
	unit s \
	print

# extract partition UUIDs

GRUB_PARTUUID=`sgdisk -i 1 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
BOOT_PARTUUID=`sgdisk -i 2 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
BOOT_UUID=`uuidgen`
ROOT0_PARTUUID=`sgdisk -i 3 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
ROOT1_PARTUUID=`sgdisk -i 4 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
ROOT2_PARTUUID=`sgdisk -i 5 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
VELOCLOUD_PARTUUID=`sgdisk -i 6 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
VELOCLOUD_UUID=`uuidgen`

# configure fstab-velocloud into fstab, if present

if [ -r "$INST_DIR/etc/config/fstab-velocloud" ]; then
	sed \
		-e "s#option device.*@ROOTDISK@1#option uuid     '$BOOT_UUID'#g" \
		-e "s#option device.*@ROOTDISK@3#option uuid     '$VELOCLOUD_UUID'#g" \
		"$INST_DIR/etc/config/fstab-velocloud" \
		> "$INST_DIR/etc/config/fstab"
fi

# copy installer trees;

BLOCK_SIZE=4096
BLOCKS="$(($INST_ROOT_SIZE * 1024 / 4))"

mkdir -p $root_dirs || exit 1
rm -rf "$OUTPUT.inst"
$STAGING_DIR_HOST/bin/genext2fs -U -i 8192 -d "$IROOT" -B "$BLOCK_SIZE" -b "$BLOCKS" "$OUTPUT.inst" -m 0 || exit 1
rm -rf "$IROOT"
$STAGING_DIR_HOST/bin/genext2fs -U -x "$OUTPUT.inst" $inst_dirs -B "$BLOCK_SIZE" -b "$BLOCKS" "$OUTPUT.inst" -m 0 -D $INCLUDE_DIR/device_table.txt || exit 1
ROOT0_UUID=`uuidgen`
$STAGING_DIR_HOST/bin/tune2fs -O extents,uninit_bg,dir_index -L root0 -U $ROOT0_UUID "$OUTPUT.inst" || exit 1
# FIXME ADD JOURNAL
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.inst"
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.inst"   # twice because the first will have added lost+found

dd if="$OUTPUT.inst" of="$OUTPUT" bs=1M seek="$ROOT0_OFF" conv=notrunc || exit 1
ROOT1_UUID=`uuidgen`
$STAGING_DIR_HOST/bin/tune2fs -L root1 -U $ROOT1_UUID "$OUTPUT.inst" || exit 1
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.inst"
dd if="$OUTPUT.inst" of="$OUTPUT" bs=1M seek="$ROOT1_OFF" conv=notrunc || exit 1
rm -rf "$OUTPUT.inst"

# exit 0

# make final boot partition;
# install grub;

BOOT_PART=hd0,gpt2
ROOT0_PART=hd0,gpt3
ROOT0_RUN="PARTUUID=$ROOT0_PARTUUID"
ROOT1_PART=hd0,gpt4
ROOT1_RUN="PARTUUID=$ROOT1_PARTUUID"
ROOT2_PART=hd0,gpt5
ROOT2_RUN="PARTUUID=$ROOT2_PARTUUID"

sed \
	-e "s#@BOOT_PART@#$BOOT_PART#g" \
	-e "s#@ROOT0_PART@#$ROOT0_PART#g" \
	-e "s#@ROOT1_PART@#$ROOT1_PART#g" \
	-e "s#@ROOT2_PART@#$ROOT2_PART#g" \
	-e "s#@ROOT0_DEV@#$ROOT0_RUN#g" \
	-e "s#@ROOT1_DEV@#$ROOT1_RUN#g" \
	-e "s#@ROOT2_DEV@#$ROOT2_RUN#g" \
	-e "s#root=@PARTUUID@##g" \
	-e "s#console=tty0 console=\([^ ]*\)#console=\1 console=tty1#g" \
	$BOOT_DIR/boot/grub/grub-velocloud.cfg \
	> $BOOT_DIR/boot/grub/grub.cfg

$STAGING_DIR_HOST/bin/grub-editenv $BOOT_DIR/boot/grub/grubenv set root1_stamp=11 root2_stamp=0

BLOCK_SIZE=4096
BLOCKS="$(($BOOT_SIZE * 1024 / 4))"

$STAGING_DIR_HOST/bin/genext2fs -U -i 8192 -d "$BOOT_DIR" -B "$BLOCK_SIZE" -b "$BLOCKS" "$OUTPUT.boot" -m 0 || exit 1
$STAGING_DIR_HOST/bin/tune2fs -O extents,uninit_bg,dir_index -L boot -U $BOOT_UUID "$OUTPUT.boot" || exit 1
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.boot"
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.boot"
dd if="$OUTPUT.boot" of="$OUTPUT" bs=1M seek="$BOOT_OFF" conv=notrunc || exit 1
rm -f "$OUTPUT.boot"

BLOCK_SIZE=4096
#VELOCLOUD_SIZE="$(($VELOCLOUD_END - $VELOCLOUD_OFF))"
VELOCLOUD_SIZE="1"
BLOCKS="$(($VELOCLOUD_SIZE * 1024 / 4))"

$STAGING_DIR_HOST/bin/genext2fs -U -i 8192 -B "$BLOCK_SIZE" -b "$BLOCKS" "$OUTPUT.user" -m 0 || exit 1
$STAGING_DIR_HOST/bin/tune2fs -O extents,uninit_bg,dir_index -L user -U $VELOCLOUD_UUID "$OUTPUT.user" || exit 1
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.user"
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.user"
dd if="$OUTPUT.user" of="$OUTPUT" bs=1M seek="$VELOCLOUD_OFF" conv=notrunc || exit 1
rm -f "$OUTPUT.user"
