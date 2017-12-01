
TARGET = ../target/bins
BRFDIR = ../bladerf/bladeRF
INCL = \
	-I. \
	-I$(BRFDIR)/host/libraries/libbladeRF/include \
	-I$(BRFDIR)/host/libraries/libbladeRF/src \
	-I$(BRFDIR)/host/build/common/include \
	-I$(BRFDIR)/host/common/include \
	-I$(BRFDIR)/firmware_common \

LIBS = \
	$(BRFDIR)/host/build/output/libbladeRF.so \
	-lfftw3 \

$(TARGET)/bladerf: bladerf.c
	cc $(INCL) -O -o $@ bladerf.c $(LIBS)

