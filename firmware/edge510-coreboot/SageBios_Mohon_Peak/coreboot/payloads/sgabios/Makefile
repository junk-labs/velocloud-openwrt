# Copyright 2010 Google Inc.
# Copyright (C) 2013 Sage Electronic Engineering, LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# $Id: Makefile 8 2010-04-22 00:03:40Z nlaredo $


$(eval $(shell bash ../xcompile.sh > .xcompile))
include .xcompile

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

BUILD_DATE = \"$(shell date -u)\"
BUILD_SHORT_DATE = \"$(shell date -u +%D)\"
BUILD_HOST = \"$(shell hostname)\"
BUILD_USER = \"$(shell whoami)\"

CFLAGS  += -Wall -Os -m32 -nostdlib -MMD

ASFLAGS += $(CFLAGS)
ASFLAGS += -DBUILD_DATE="$(BUILD_DATE)"
ASFLAGS += -DBUILD_SHORT_DATE="$(BUILD_SHORT_DATE)"
ASFLAGS += -DBUILD_HOST="$(BUILD_HOST)"
ASFLAGS += -DBUILD_USER="$(BUILD_USER)"

LDSCRIPT := rom16.ld
LDFLAGS := -T $(LDSCRIPT) -nostdlib
OBJCOPY := objcopy
RM := rm

ASRCS = sgabios.S

CSRCS =

SRCS = $(CSRCS) $(ASRCS)

OBJS = ${CSRCS:.c=.o} ${ASRCS:.S=.o}
INCS = ${CSRCS:.c=.h} ${ASRCS:.S=.h}

PROGS = sgabios.bin csum8

.SUFFIXES: .bin .elf
.PHONY: buildinfo distclean
.NOTPARALLEL: all .depend

all: $(PROGS)

%.o:%.S
	@printf "    CC         $@\n"
	$(Q)$(CC) $(ASFLAGS) -c -o $@ $<

sgabios.bin: sgabios.elf
	@printf "    OBJCOPY    $@\n"
	$(Q)$(OBJCOPY) -O binary $< $@
	@printf "    CSUM8      append checksum\n"
	./csum8 $@

sgabios.elf: .depend $(OBJS) $(LDSCRIPT) csum8
	@printf "    LINK       $@\n"
	$(Q)$(LD) $(LDFLAGS) $(OBJS) -o $@

csum8: csum8.c
	@printf "    HOSTCC     $@\n"
	$(Q)$(HOSTCC) -MMD -Wall -O2 -o $@ $<

sgabios.o: buildinfo

buildinfo:
	$(Q)touch sgabios.S

distclean: clean
	$(Q)$(RM) -f .depend .xcompile

clean:
	$(Q)$(RM) -f $(PROGS) $(OBJS) *.elf *.srec *.com version.h *.d

.depend:: $(INCS) $(SRCS) Makefile
	$(Q)$(RM) -f .depend
	$(Q)$(CPP) -M $(CFLAGS) $(SRCS) >.tmpdepend && mv .tmpdepend .depend

ifeq (.depend, $(wildcard .depend))
include .depend
else
# if no .depend file existed, add a make clean to the end of building .depend
.depend::
	$(Q)$(MAKE) clean
endif
