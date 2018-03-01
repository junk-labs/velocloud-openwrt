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

OUTPUTBASE="${OUTPUT//-usb-ext4.img}"
if [ "${OUTPUTBASE}" != "$OUTPUT" -a -z "${ENV_CONFIG_TARGET_NO_EMBEDDED}" ]; then
    EMBEDDED="${OUTPUT//-usb-ext4.img}-embedded-ext4.tar.bz2"
else
    EMBEDDED=""
fi

# translate install images dirs;
# first inst_dir entry must point to a dir without ':' separator;

IROOT="${OUTPUT}.iroot"
INST_PATH="images"

#root_dirs=""
#inst_dirs=""
copy_dirs=""
i=6
while [ $i -lt $# ]
do
	arg=${ARGV[$i]}
	base=`basename $arg`
	#root_dirs="$root_dirs ${IROOT}/${INST_PATH}/${base}"
	#inst_dirs="$inst_dirs -d ${arg}:/${INST_PATH}/${base}"
	copy_dirs="$copy_dirs ${arg}"
	i=$(($i + 1))
done

rm -f "$OUTPUT"
rm -rf $IROOT

INST_SIZE=`du -msc $copy_dirs 2>/dev/null | tail -1 | awk '{print $1;}'`
# add 5%
INST_SIZE=$((INST_SIZE*105/100))
# round up to multiple of 100 MB
INST_SIZE=$(((INST_SIZE+99)/100*100))

mkdir -p $IROOT/$INST_PATH
cp -a $copy_dirs $IROOT/$INST_PATH

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
INST_END="$(($INST_OFF + $INST_SIZE))"

TAIL_OFF=$INST_END
TAIL_SIZE=1
TAIL_END="$(($TAIL_OFF + $TAIL_SIZE))"

# copy rootfs image;

dd if="$ROOT_FSIMAGE" of="$OUTPUT" bs=1M seek="$ROOT_OFF" conv=notrunc count="$ROOT_SIZE" || exit 1
dd if=/dev/zero of="$OUTPUT" bs=1M seek="$TAIL_OFF" conv=notrunc count="$TAIL_SIZE" || exit 1

# copy installer trees;

BLOCK_SIZE=4096
BLOCKS="$((($INST_END - $INST_OFF) * 1024 / 4))"

# mkdir -p $root_dirs || exit 1
echo $INST_ROOT_SIZE > $IROOT/$INST_PATH/root-size
echo $GRUB2_MODULES > $IROOT/$INST_PATH/grub-modules

$STAGING_DIR_HOST/bin/make_ext4fs -i 8192 -l $((BLOCKS*BLOCK_SIZE)) -m 0 "$OUTPUT.inst" "$IROOT"
if [ ! -z "$EMBEDDED" ]; then
    # Generate an "installer tarball" for Dolphin and newer hardware platforms
    if [ -r $IROOT/$INST_PATH/root-x64/root/installer ]; then
        cp -a $IROOT/$INST_PATH/root-x64/root/installer $IROOT
        tar --numeric-owner --owner=0 --group=0 -cjf "$EMBEDDED" -C $IROOT installer $INST_PATH
    fi
fi
rm -rf "$IROOT"
INST_UUID=`uuidgen`
$STAGING_DIR_HOST/bin/tune2fs -U $INST_UUID "$OUTPUT.inst" || exit 1
$STAGING_DIR_HOST/bin/e2fsck -fy "$OUTPUT.inst"

dd if="$OUTPUT.inst" of="$OUTPUT" bs=1M seek="$INST_OFF" conv=notrunc || exit 1
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

BLOCK_SIZE=4096
BLOCKS="$(($BOOT_SIZE * 1024 / 4))"

$STAGING_DIR_HOST/bin/make_ext4fs -i 8192 -l $((BLOCKS*BLOCK_SIZE)) -m 0 "$OUTPUT.boot" "$BOOT_DIR"
dd if="$OUTPUT.boot" of="$OUTPUT" bs=1M seek="$BOOT_OFF" conv=notrunc || exit 1
rm -f "$OUTPUT.boot"

