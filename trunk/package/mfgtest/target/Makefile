# Makefile for bladerf server and client;

# test profile config;
PROFILE_EM7455=profile.em7455
PROFILE_EM7430=profile.em7430
COLO_EM7455=colo.em7455
COLO_EM7430=colo.em7430
TTY=/dev/ttyUSB2

# build machine;
BUILD=sandra@10.1.2:bladerf

# SDR run settings;
XDIR=/home/velocloud/bladerf
LDPATH=LD_LIBRARY_PATH=$(XDIR)/bins
RRPRI=taskset -c 1 chrt 99

# minowboard/SDR machine;
MINIP=10.0.1.11
MINSDR=velocloud@$(MINIP):$(XDIR)
SSHOPTS=ssh -l velocloud -i id_rsa

# macro to set cpu performance;

cpugov=/sys/devices/system/cpu/cpu$1/cpufreq/scaling_governor
vcid=$(shell cat /sys/class/dmi/id/product_serial)
log=/var/log/sdrsvr.log

FW_FPGA=firmware/hostedx40-latest.rbf
FW_BLADERF=firmware/bladeRF_fw_v1.9.1.img

define perf
	echo performance > $(call cpugov,0)
	echo performance > $(call cpugov,1)
	if [ -f $(call cpugov,2) ]; then \
		echo powersave > $(call cpugov,2); \
		echo powersave > $(call cpugov,3); \
	fi
endef

# install system files;

inst:
	cp id_rsa.pub ~/.ssh/authorized_keys
	chown velocloud.velocloud ~/.ssh/authorized_keys
	cp sdrsvr.rules /etc/udev/rules.d/
	cp bladerf@.service /etc/systemd/system/
	chown 0.0 udev.sh /etc/udev/rules.d/sdrsvr.rules /etc/systemd/system/bladerf@.service
	udevadm control --reload
	systemctl daemon-reload
	echo "installed $$(date)" >> $(log)

# set cpu performance;
# to be run only on minnowboard SDR;

perf:
	$(call perf)

# calibrate both default and our fine;
# to be run only on minnowboard SDR;

cal: perf
	$(LDPATH) ./bins//bladeRF-cli -e 'cal table dc rx'
	$(LDPATH) $(RRPRI) ./bins/bladerf -c -v

# run bladerf server on minnowboard;

brf: perf
	$(LDPATH) $(RRPRI) ./bins/bladerf

# same but very verbose;

brf-v: perf
	$(LDPATH) BLADERF_LOG_LEVEL=verbose $(RRPRI) ./bins/bladerf -v

# udev triggered;

brf-start: perf
	echo "==> starting sdr server" >> $(log)
	#$(LDPATH) $(RRPRI) nohup strace -tt -o /tmp/strace.brf ./bins/bladerf -d 10 >> $(log) 2>&1 &
	$(LDPATH) $(RRPRI) ./bins/bladerf -d 10 >> $(log) 2>&1

brf-stop:
	echo "==> stopping sdr server" >> $(log)
	if [ -n "$$(pgrep bladerf)" ]; then \
		kill -9 $$(pgrep bladerf); \
	fi

# rsync in from build machine;
# to be run on DUT, edge510 console;
# WAN link to build machine;

rsi:
	rsync -avH $(BUILD)/ .

# rsync from DUT edge510 to minnowboard;
# to be run on DUT, edge510 console;
# LAN link to minnowboard/SDR at 10.0.1.11;

rso:
	rsync -avH -e "$(SSHOPTS)" . $(MINSDR)/

# run DUT cellular radio test;
# use minnowboard SDR server;

test-em7455:
	-./bins/swtest -t $(TTY) -f $(PROFILE_EM7455) -N $(vcid) -B $(MINIP)
	rsync -avH -e "$(SSHOPTS)" plots/ $(MINSDR)/

test-em7430:
	-./bins/swtest -t $(TTY) -f $(PROFILE_EM7430) -N $(vcid) -B $(MINIP)
	rsync -avH -e "$(SSHOPTS)" plots/ $(MINSDR)/

# run DUT cellular test manually;
# for co-lo testing, manually step through profiles;
# or use command line options;

colo-7455:
	-./bins/swtest -t $(TTY) -f $(COLO_EM7455) -N $(vcid)

colo-7430:
	-./bins/swtest -t $(TTY) -f $(COLO_EM7430) -N $(vcid)

# copy plots out to minnowboard;

po:
	rsync -avH -e "$(SSHOPTS)" plots/ $(MINSDR)/plots/

# update fpga firmware;

fw-fpga:
	$(LDPATH) ./bins/bladeRF-cli -L $(FW_FPGA)

# update SDR processor firmware;

fw-sdr:
	$(LDPATH) ./bins/bladeRF-cli -f $(FW_BLADERF)

# help

help:
	@echo " cal          - do calibration"
	@echo " brf-stop     - stop SDR server"
	@echo " brf-start    - start SDR server"
	@echo " fw-fpga      - flash SDR fpga firmware"
	@echo " fw-bladerf   - flash SDR arm firmware"
	@echo " test-em7455  - test with SDR, expecting EM7455"
	@echo " test-em7430  - test with SDR, expecting EM7430"

