#!/bin/sh

VER="$1"
CHANGELOG="$2"
DATE=$(date -R)

sed -i -e "1i\\
quagga ($VER) unstable; urgency=medium\\
\\
  * Dummy build line to fake out debuild with correct version\\
\\
 -- VeloCloud Build System <build@velocloud.net> $DATE\\
\\
" $CHANGELOG
