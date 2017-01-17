# script to build all flavors of Sagebios for RCCVE #

DESTDIR=/home/wenwang/adi_coreboot_public/releases/ADI_RCCVE-01.00.00.10
# DESTDIR=./releases/ADI_RCCVE-01.00.00.04
IMG_RCCVE=ADI_RCCVE-01.00.00.10.rom
IMG_RCCVE_NODEBUG=ADI_RCCVE-01.00.00.10-nodebug.rom
IMG_RCCVE_NOSGABIOS=ADI_RCCVE-01.00.00.10-nosgabios.rom
IMG_RCCVE_NODEBUG_NOSGABIOS=ADI_RCCVE-01.00.00.10-nodebug-nosgabios.rom
IMG_RCCVE_TPM=ADI_RCCVE-TPM-01.00.00.10.rom
IMG_RCCVE_TPM_NODEBUG=ADI_RCCVE-TPM-01.00.00.10-nodebug.rom
IMG_DFF2=ADI_DFF2-01.00.00.10.rom
IMG_DFF2_NODEBUG=ADI_DFF2-01.00.00.10-nodebug.rom

# 0. build sortboot order payload
./build_sortbootorder.sh > sortbootorder_build.log 2>&1
cp sortbootorder_build.log  $DESTDIR/$IMG_RCCVE.build.log 

# 1.  use config.rccve config
echo "Building RCCVE Sagebios using config.rccve "
rm .config
cp configs/config.rccve .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE
cp rccve.log $DESTDIR/$IMG_RCCVE.build.log
rm make.log
rm map.log
rm rccve.log

# 2.  use config.rccve.nodebug config
echo "Building RCCVE Sagebios using config.rccve.nodebug "
rm .config
cp configs/config.rccve.nodebug .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE_NODEBUG
cp rccve.log $DESTDIR/$IMG_RCCVE_NODEBUG.build.log
rm make.log
rm map.log
rm rccve.log

# 3.  use config.rccve.nosgabios config
echo "Building RCCVE Sagebios using config.rccve.nosgabios "
rm .config
cp configs/config.rccve.nosgabios .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE_NOSGABIOS
cp rccve.log $DESTDIR/$IMG_RCCVE_NOSGABIOS.build.log
rm make.log
rm map.log
rm rccve.log

# 4. use config.rccve.nodebug.nosgabios config
echo "Building RCCVE Sagebios using config.rccve.nodebug.nosgabios "
rm .config
cp configs/config.rccve.nodebug.nosgabios .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE_NODEBUG_NOSGABIOS
cp rccve.log $DESTDIR/$IMG_RCCVE_NODEBUG_NOSGABIOS.build.log
rm make.log
rm map.log
rm rccve.log



# 5.  use config.dff2 config
echo "Building DFF2 Sagebios using config.dff2 "
rm .config
cp configs/config.dff2 .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding dff2 boot order .."

./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f dff2_bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f dff2_bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log >  dff2.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_DFF2
cp dff2.log $DESTDIR/$IMG_DFF2.build.log
rm make.log
rm map.log
rm dff2.log

# 6.  use config.dff2.nodebug config
echo "Building DFF2 Sagebios using config.dff2.nodebug "
rm .config
cp configs/config.dff2.nodebug .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding dff2 boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f dff2_bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f dff2_bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > dff2.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_DFF2_NODEBUG
cp dff2.log $DESTDIR/$IMG_DFF2_NODEBUG.build.log
rm make.log
rm map.log
rm dff2.log


# 7.  use config.rccve-tpm config
echo "Building RCCVE Sagebios using config.rccve "
rm .config
cp configs/config.rccve-tpm .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE_TPM
cp rccve.log $DESTDIR/$IMG_RCCVE_TPM.build.log
rm make.log
rm map.log
rm rccve.log

# 8.  use config.rccve-tpm.nodebug config
echo "Building RCCVE Sagebios using config.rccve.nodebug "
rm .config
cp configs/config.rccve-tpm.nodebug .config
rm -rf build
rm -rf payloads/seabios/out
make clean
make > make.log 2>&1
echo "adding boot order .."
./build/cbfstool build/coreboot.rom add-payload -f payloads/sortbootorder/sortbootorder.elf -n img/"setup" -c LZMA
./build/cbfstool build/coreboot.rom add -f bootorder_def -n bootorder -t raw -b 0xfff00000
./build/cbfstool build/coreboot.rom add -f bootorder_map -t raw -n bootorder_map
./build/cbfstool build/coreboot.rom add -f bootorder_def -t raw -n bootorder_def
./build/cbfstool build/coreboot.rom print > map.log
cat make.log map.log > rccve.log
echo "copy image to destinaton directory"
cp build/coreboot.rom $DESTDIR/$IMG_RCCVE_TPM_NODEBUG
cp rccve.log $DESTDIR/$IMG_RCCVE_TPM_NODEBUG.build.log
rm make.log
rm map.log
rm rccve.log









