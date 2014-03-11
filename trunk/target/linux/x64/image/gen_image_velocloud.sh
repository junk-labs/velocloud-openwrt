#!/usr/bin/env bash
# Copyright (C) 2013 Veloccloud.net
set -x
[ $# -ge 7 ] || {
    echo "SYNTAX: $0 <file> <boot size> <boot dir> <rootfs size> <rootfs image> <install root size> <install dirs> ..."
    exit 1
}

ARGV=("$@")
OUTPUT="$1"
BOOT_SIZE="$2"
BOOT_DIR="$3"
ROOT_SIZE="$4"
ROOT_FSIMAGE="$5"
INST_ROOT_SIZE="$6"
INST_DIR="$7"

# translate install images dirs;
# first inst_dir entry must point to a dir without ':' separator;

IROOT="${OUTPUT}.iroot"
INST_PATH="images"

root_dirs=""
inst_dirs=""
i=6
while [ $i -lt $# ]
do
	arg=${ARGV[$i]}
	base=`basename $arg`
	root_dirs="$root_dirs ${IROOT}/${INST_PATH}/${base}"
	inst_dirs="$inst_dirs -d ${arg}:/${INST_PATH}/${base}"
	i=$(($i + 1))
done

rm -f "$OUTPUT"
rm -rf $IROOT

# align all partition to 1MB boundaries;
# root and installer filesystem are same size;
# INST_ROOT_SIZE is a parameter to the installer;

GRUB_OFF=1
GRUB_END=2

BOOT_OFF=$GRUB_END
BOOT_END="$(($BOOT_OFF + $BOOT_SIZE))"

ROOT_OFF=$BOOT_END
ROOT_END="$(($ROOT_OFF + $ROOT_SIZE))"

INST_OFF=$ROOT_END
INST_END="$(($INST_OFF + $ROOT_SIZE))"

TAIL_OFF=$INST_END
TAIL_SIZE=1
TAIL_END="$(($TAIL_OFF + $TAIL_SIZE))"

# copy rootfs image;

dd if="$ROOT_FSIMAGE" of="$OUTPUT" bs=1M seek="$ROOT_OFF" conv=notrunc count="$ROOT_SIZE"
dd if=/dev/zero of="$OUTPUT" bs=1M seek="$TAIL_OFF" conv=notrunc count="$TAIL_SIZE"

# copy installer trees;

BLOCKS="$(($ROOT_SIZE * 1024))"

mkdir -p $root_dirs
echo $INST_ROOT_SIZE > $IROOT/$INST_PATH/root-size
echo $GRUB2_MODULES > $IROOT/$INST_PATH/grub-modules
$STAGING_DIR_HOST/bin/genext2fs -U -i 8192 -d "$IROOT" -b "$BLOCKS" "$OUTPUT.inst"
rm -rf "$IROOT"
$STAGING_DIR_HOST/bin/genext2fs -U -x "$OUTPUT.inst" $inst_dirs -b "$BLOCKS" "$OUTPUT.inst"
INST_UUID=`uuidgen`
$STAGING_DIR_HOST/bin/tune2fs -O extents,uninit_bg,dir_index -U $INST_UUID "$OUTPUT.inst"
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.inst"

dd if="$OUTPUT.inst" of="$OUTPUT" bs=1M seek="$INST_OFF" conv=notrunc
rm -rf "$OUTPUT.inst"

# create gpt partition table;

parted -s -- $OUTPUT \
	unit MiB \
	mklabel gpt \
	mkpart grub $GRUB_OFF $GRUB_END \
	set 1 bios_grub on \
	mkpart boot $BOOT_OFF $BOOT_END \
	set 2 boot on \
	mkpart root $ROOT_OFF $ROOT_END \
	set 3 boot off \
	mkpart inst $INST_OFF $INST_END \
	set 4 boot off \
	print \
	unit s \
	print

# extract root partition uuid;
# overwrite root=PARTUUID with root uuid;

ROOT_UUID=`sgdisk -i 3 $OUTPUT | grep 'unique GUID' | sed -e 's/^.*: //'`
sed -i \
	-e "s/@PARTUUID@/PARTUUID=$ROOT_UUID/g" \
	-e "s/vc.install/vc.install=$INST_UUID/" \
	$BOOT_DIR/boot/grub/grub.cfg

# make final boot partition;
# install grub;

BLOCKS="$(($BOOT_SIZE * 1024))"

$STAGING_DIR_HOST/bin/genext2fs -U -i 8192 -d "$BOOT_DIR" -b "$BLOCKS" "$OUTPUT.boot"
dd if="$OUTPUT.boot" of="$OUTPUT" bs=1M seek="$BOOT_OFF" conv=notrunc
rm -f "$OUTPUT.boot"

