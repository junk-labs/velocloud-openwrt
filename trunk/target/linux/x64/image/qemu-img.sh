#!/usr/bin/env bash

# Gruesome workaround for problems (as yet not understood) with openwrt
# "host" tools that require glib. We see mysterious crashes when the
# program is run wuth an LD_LIBRARY_PATH pointing to "openwrt host"
# libraries (with some indications of a size_t size mismatch). And if
# run using the ld-linux.so from the OpenWrt "host" tools, it's even
# worse: a mysterious duplicate call to a .init section.

# So this just simplifies things by invoking the "real" binary without
# any LD_LIBRARY_PATH (i.e. run the openwrt host binary tool against
# the build platform native shared libraries.  This works the best of
# all (oddly enough) - it's at least more robust than the more "correct"
# alternatives above.

# Slight complication: when building in-tree, the host tools are run
# out of STAGING_DIR/host/bin and .../lib.  But when run from an
# ImageBuilder, it's run via a wrapper script in .../host/bin, and that
# picks up the binary from .../host/bin/bundled.

if [ -z "$STAGING_DIR_HOST" ]; then
    STAGING_DIR_HOST=$(dirname $0)/../../../../staging_dir/host
fi
if [ -e "$STAGING_DIR_HOST"/bin/bundled/qemu-img ]; then
    #QLIB="$STAGING_DIR_HOST"/bin/bundled/lib
    QBIN="$STAGING_DIR_HOST"/bin/bundled
    PFX=""
elif [ -e "$STAGING_DIR_HOST"/bin/qemu-img ]; then
    QLIB="$STAGING_DIR_HOST"/lib
    QBIN="$STAGING_DIR_HOST"/bin
    PFX="env LD_LIBRARY_PATH=$QLIB"
else
    1>&2 echo Cannot find qemu-img under "$STAGING_DIR_HOST"
    exit 1
fi

#env LD_LIBRARY_PATH="$QLIB" "$QBIN"/qemu-img "$@"
$PFX "$QBIN"/qemu-img "$@"
