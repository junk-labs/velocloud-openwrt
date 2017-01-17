# script to build all flavors of Sagebios for DFF2 #

DESTDIR=./release
# DESTDIR=./releas/ADI_DFF2-01.00.00.04
IMG_DFF2=ADI_DFF2-01.00.00.05-Testdrop.rom
IMG_DFF2_NODEBUG=ADI_DFF2-01.00.00.05-nodebug-Testdrop.rom




# 1.  use config.dff2 config
echo "Building DFF2 Sagebios using config.dff2 "
rm .config
cp configs/config.dff2 .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add -f dff2bootlist.txt -n bootorder -t raw
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log >  dff2.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_DFF2
cp dff2.log $DESTDIR/$IMG_DFF2.build.log
rm make.log
rm map.log
rm dff2.log

# 2.  use config.dff2.nodebug config
echo "Building DFF2 Sagebios using config.dff2.nodebug "
rm .config
cp configs/config.dff2.nodebug .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add -f dff2bootlist.txt -n bootorder -t raw
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > dff2.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_DFF2_NODEBUG
cp dff2.log $DESTDIR/$IMG_DFF2_NODEBUG.build.log
rm make.log
rm map.log
rm dff2.log



