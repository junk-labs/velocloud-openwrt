# Usage: ./gen_ova.sh /path/to/disk.vmdk /path/to/template.ovf

VMDKPATH="$1"
OVFTEMPLATE="$2"

if [ ! -r "$VMDKPATH" ]; then
	1>&2 echo Cannot read VMDK file "$VMDKPATH"
	1>&2 echo Usage: ./gen_ova.sh /path/to/disk.vmdk /path/to/template.ovf
	exit 1
elif [ ! -r "$OVFTEMPLATE" ]; then
	1>&2 echo Cannot read OVF template "$OVFTEMPLATE"
	1>&2 echo Usage: ./gen_ova.sh /path/to/disk.vmdk /path/to/template.ovf
	exit 1
fi

VMDKDIR=`dirname "$VMDKPATH"`
VDISKMGR=$STAGING_DIR_HOST/bin/vmware-vdiskmanager

OVFBASE=`basename "$OVFTEMPLATE" .ovf`

GENDISKNAME=$OVFBASE-disk1.vmdk
GENOVFNAME=$OVFBASE.ovf
GENMFNAME=$OVFBASE.mf
GENOVANAME=`basename "$VMDKPATH" .vmdk`.ova

FULLVERSION=${IMG_PREFIX#*$ARCH-}
VERSION=${FULLVERSION%%-*}

# Generate a stream-optimized and compressed VMDK for the OVF
rm -f $VMDKDIR/$GENDISKNAME
env LD_LIBRARY_PATH=$STAGING_DIR_HOST/lib \
	$VDISKMGR -t 5 -r $VMDKPATH $VMDKDIR/$GENDISKNAME

DISKSIZE=`stat -c %s $VMDKDIR/$GENDISKNAME`
sed \
	-e "s/@DISKNAME@/$GENDISKNAME/g" \
	-e "s/@DISKSIZE@/$DISKSIZE/g" \
	-e "s/@VERSION@/$VERSION/g" \
	-e "s/@FULLVERSION@/$FULLVERSION/g" \
	$OVFTEMPLATE > "$VMDKDIR/$GENOVFNAME"

OVFSUM=`sha1sum -b "$VMDKDIR/$GENOVFNAME" | awk '{print $1;}'`
DISKSUM=`sha1sum -b "$VMDKDIR/$GENDISKNAME" | awk '{print $1;}'`
cat > "$VMDKDIR/$GENMFNAME" <<-EOF
SHA1($GENOVFNAME)= $OVFSUM
SHA1($GENDISKNAME)= $DISKSUM
EOF

tar --numeric-owner --owner=0 --group=0 -C $VMDKDIR -cvf $VMDKDIR/$GENOVANAME $GENOVFNAME $GENMFNAME $GENDISKNAME
rm -f $GENOVFNAME $GENMFNAME $GENDISKNAME
