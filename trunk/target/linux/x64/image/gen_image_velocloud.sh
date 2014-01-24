#!/usr/bin/env bash
# Copyright (C) 2013 Veloccloud.net
set -x
[ $# -ge 8 ] || {
    echo "SYNTAX: $0 <file> <align> <boot size> <boot dir> <rootfs size> <rootfs image> <install root size> <install dirs> ..."
    exit 1
}

ARGV=("$@")
OUTPUT="$1"
ALIGN="$2"
BOOT_SIZE="$3"
BOOT_DIR="$4"
ROOT_SIZE="$5"
ROOT_FSIMAGE="$6"
INST_ROOT_SIZE="$7"
INST_DIR="$8"

# translate install images dirs;
# first inst_dir entry must point to a dir without ':' separator;

IROOT="${OUTPUT}.iroot"
INST_PATH="images"

root_dirs=""
inst_dirs=""
i=7
while [ $i -lt $# ]
do
	arg=${ARGV[$i]}
	base=`basename $arg`
	root_dirs="$root_dirs ${IROOT}/${INST_PATH}/${base}"
	inst_dirs="$inst_dirs -d ${arg}:/${INST_PATH}/${base}"
	i=$(($i + 1))
done

rm -f "$OUTPUT"

# fix to align to 1MB boundaries;

head=32
sect=64
cyl=$(( ($BOOT_SIZE + $ROOT_SIZE) * 1024 * 1024 / ($head * $sect * 512)))

# create partition table
# the installer uses plain old MSDOS format;
# root and installer filesystem are same size;
# INST_ROOT_SIZE is a parameter to the installer;

set `ptgen -o "$OUTPUT" -h $head -s $sect -p ${BOOT_SIZE}m -p ${ROOT_SIZE}m -p ${ROOT_SIZE}m ${ALIGN:+-l $ALIGN}`

BOOT_OFFSET="$(($1 / 512))"
BOOT_SIZE="$(($2 / 512))"
ROOT_OFFSET="$(($3 / 512))"
ROOT_SIZE="$(($4 / 512))"
INST_OFFSET="$(($5 / 512))"
INST_SIZE="$(($6 / 512))"

# copy rootfs image;

dd if="$ROOT_FSIMAGE" of="$OUTPUT" bs=512 seek="$ROOT_OFFSET" conv=notrunc count="$ROOT_SIZE"

# copy installer trees;

BLOCKS="$(($INST_SIZE / 2))"

mkdir -p $root_dirs
echo $INST_ROOT_SIZE > $IROOT/$INST_PATH/root-size
echo $GRUB2_MODULES > $IROOT/$INST_PATH/grub-modules
genext2fs -U -i 8192 -d "$IROOT" -b "$BLOCKS" "$OUTPUT.inst"
rm -rf "$IROOT"

genext2fs -U -x "$OUTPUT.inst" $inst_dirs -b "$BLOCKS" "$OUTPUT.inst"
dd if="$OUTPUT.inst" of="$OUTPUT" bs=512 seek="$INST_OFFSET" conv=notrunc
rm -rf "$OUTPUT.inst"

# install grub;

[ -n "$NOGRUB" ] && exit 0

BLOCKS="$(($BOOT_SIZE / 2))"

genext2fs -U -i 8192 -d "$BOOT_DIR" -b "$BLOCKS" "$OUTPUT.boot"
dd if="$OUTPUT.boot" of="$OUTPUT" bs=512 seek="$BOOT_OFFSET" conv=notrunc
rm -f "$OUTPUT.boot"

