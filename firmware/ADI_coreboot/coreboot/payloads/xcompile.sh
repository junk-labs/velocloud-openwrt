#!/bin/sh
#
# This file is part of the coreboot project.
#
# Copyright (C) 2007-2010 coresystems GmbH
# Copyright (C) 2013 Sage Electronic Engineering, LLC
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
#

testcc()
{
	echo "_start(void) {}" > .$$$$.c
	$1 -nostdlib -Werror $2 .$$$$.c -o .$$$$.tmp 2>/dev/null >/dev/null
	ret=$?
	rm -f .$$$$.c .$$$$.tmp
	return $ret
}

for make in make gmake gnumake; do
	if [ "`$make --version 2>/dev/null | grep -c GNU`" -gt 0 ]; then
		MAKE=$make
		break
	fi
done

GCCPREFIX=invalid

if [ "$SAGE_HOME" == "" ]; then
	echo '$(error ERROR: SageEDK build tools were not found.  Please visit http://www.se-eng.com/downloads.html or contact sales@se-eng.com to obtain a copy of the SageEDK.)'
	exit 1
fi

# Search for the SageBIOS toolchain where it is installed for the SageEDK.
if [[ $OSTYPE == "cygwin" && "$SAGE_HOME" != "" ]]; then
	# Convert $SAGE_HOME from Windows style path, if necessary
	SAGE_EDK_DIR=$(cygpath --unix $SAGE_HOME)
else
	SAGE_EDK_DIR=$SAGE_HOME
fi
SAGE_EDK_TOOLS="$SAGE_EDK_DIR/tools/xgcc/bin/"

TMPFILE=`mktemp /tmp/temp.XXXX 2>/dev/null || echo /tmp/temp.78gOIUGz`
touch $TMPFILE

# This loops over all supported architectures in TARCH
TARCH=('i386' 'x86_64')
TWIDTH=32
for search_for in "${TARCH[@]}"; do
	TARCH_SEARCH=("${TARCH_SEARCH[@]}" ${SAGE_EDK_TOOLS}${search_for}-elf- ${search_for}-elf-)
done
echo '# TARCH_SEARCH='${TARCH_SEARCH[@]}

for gccprefixes in "${TARCH_SEARCH[@]}" ""; do
	if ! which ${gccprefixes}as 2>/dev/null >/dev/null; then
		continue
	fi
	rm -f ${TMPFILE}.o
	if ${gccprefixes}as -o ${TMPFILE}.o ${TMPFILE}; then
		TYPE=`${gccprefixes}objdump -p ${TMPFILE}.o`
		if [ ${TYPE##* } == "elf${TWIDTH}-${TARCH}" ]; then
			GCCPREFIX=$gccprefixes
			ASFLAGS=
			CFLAGS=
			LDFLAGS=
			break
		fi
	fi
	if ${gccprefixes}as --32 -o ${TMPFILE}.o ${TMPFILE}; then
		TYPE=`${gccprefixes}objdump -p ${TMPFILE}.o`
		if [ ${TYPE##* } == "elf${TWIDTH}-${TARCH}" ]; then
			GCCPREFIX=$gccprefixes
			ASFLAGS=--32
			CFLAGS="-m32 -Wl,-b,elf32-i386 -Wl,-melf_i386 "
			LDFLAGS="-b elf32-i386 -melf_i386"
			break
		fi
	fi
done
rm -f $TMPFILE ${TMPFILE}.o

if [ "$GCCPREFIX" = "invalid" ]; then
	echo '$(error ERROR: Toolchain is invalid.  Please reinstall a licensed copy of the SageEDK or contact support@se-eng.com.)'
	exit 1
fi

CC="${GCCPREFIX}gcc"
testcc "$CC" "$CFLAGS-Wa,--divide " && CFLAGS="$CFLAGS-Wa,--divide "
testcc "$CC" "$CFLAGS-fno-stack-protector " && CFLAGS="$CFLAGS-fno-stack-protector "
testcc "$CC" "$CFLAGS-Wl,--build-id=none " && CFLAGS="$CFLAGS-Wl,--build-id=none "
# GCC 4.6 is much more picky about unused variables. Turn off it's warnings for
# now:
testcc "$CC" "$CFLAGS-Wno-unused-but-set-variable " && \
	       CFLAGS="$CFLAGS-Wno-unused-but-set-variable "

for gccprefixes in ${SAGE_EDK_TOOLS}; do
	if [ "`${gccprefixes}iasl 2>/dev/null | grep -c ACPI`" -gt 0 ]; then
		IASL=${gccprefixes}iasl
		break
	else
		echo '$(error ERROR: Toolchain is invalid.  Please reinstall a licensed copy of the SageEDK or contact support@se-eng.com.)'
		exit 1
	fi
done

if which gcc 2>/dev/null >/dev/null; then
	HOSTCC=gcc
else
	HOSTCC=cc
fi

cat << EOF
# elf${TWIDTH}-${TARCH} toolchain
GCCPREFIX=${GCCPREFIX}

export AS:=${GCCPREFIX}as ${ASFLAGS}
export CC:=${GCCPREFIX}gcc ${CFLAGS}
export CPP:=${GCCPREFIX}cpp
export AR:=${GCCPREFIX}ar
export LD:=${GCCPREFIX}ld ${LDFLAGS}
export STRIP:=${GCCPREFIX}strip
export NM:=${GCCPREFIX}nm
export OBJCOPY:=${GCCPREFIX}objcopy
export OBJDUMP:=${GCCPREFIX}objdump

export IASL:=${IASL}

# native toolchain
export HOSTCC:=${HOSTCC}
EOF