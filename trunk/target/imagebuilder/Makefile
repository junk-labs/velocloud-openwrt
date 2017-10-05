# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/host.mk
include $(INCLUDE_DIR)/version.mk

override MAKEFLAGS=

PKG_OS:=$(word 2,$(subst -, ,$(shell $(HOSTCC) -dumpmachine)))
PKG_CPU:=$(word 1,$(subst -, ,$(shell $(HOSTCC) -dumpmachine)))

IB_NAME:=OpenWrt-ImageBuilder-$(BOARD)$(if $(SUBTARGET),_$(SUBTARGET))-for-$(PKG_OS)-$(PKG_CPU)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(IB_NAME)
IB_KDIR:=$(patsubst $(TOPDIR)/%,$(PKG_BUILD_DIR)/%,$(KERNEL_BUILD_DIR))
IB_LDIR:=$(patsubst $(TOPDIR)/%,$(PKG_BUILD_DIR)/%,$(LINUX_DIR))
IB_DTSDIR:=$(patsubst $(TOPDIR)/%,$(PKG_BUILD_DIR)/%,$(LINUX_DIR))/arch/$(ARCH)/boot/dts/

all: compile

$(BIN_DIR)/$(IB_NAME).tar.bz2: clean
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(IB_KDIR) $(IB_LDIR) $(PKG_BUILD_DIR)/staging_dir/host/lib \
		$(PKG_BUILD_DIR)/target $(PKG_BUILD_DIR)/scripts $(IB_DTSDIR)
	-cp $(TOPDIR)/.config $(PKG_BUILD_DIR)/.config
	$(CP) \
		$(INCLUDE_DIR) $(SCRIPT_DIR) \
		$(TOPDIR)/rules.mk \
		./files/Makefile \
		./files/repositories.conf \
		$(TMP_DIR)/.targetinfo \
		$(TMP_DIR)/.packageinfo \
		$(PKG_BUILD_DIR)/
	$(VERSION_SED) $(PKG_BUILD_DIR)/repositories.conf
	$(CP) $(PACKAGE_DIR) $(PKG_BUILD_DIR)/packages
	$(CP) $(TOPDIR)/target/linux $(PKG_BUILD_DIR)/target/
	if [ -d $(TOPDIR)/staging_dir/host/lib/grub ]; then \
		$(CP) $(TOPDIR)/staging_dir/host/lib/grub/ $(PKG_BUILD_DIR)/staging_dir/host/lib; \
	fi
	rm -rf \
		$(PKG_BUILD_DIR)/target/linux/*/files{,-*} \
		$(PKG_BUILD_DIR)/target/linux/*/patches{,-*}
	-cp $(KERNEL_BUILD_DIR)/* $(IB_KDIR)/ # don't copy subdirectories here
	[ -d $(KERNEL_BUILD_DIR)/inst.grub ] && $(CP) $(KERNEL_BUILD_DIR)/inst.grub $(IB_KDIR)/ || true # except inst.grub
	-cp $(LINUX_DIR)/.config $(IB_LDIR)/
	-$(SCRIPT_DIR)/bundle-libraries.sh $(IB_LDIR)/scripts/dtc \
	  $(LINUX_DIR)/scripts/dtc/dtc
	if [ -d $(LINUX_DIR)/arch/$(ARCH)/boot/dts ]; then \
		$(CP) $(LINUX_DIR)/arch/$(ARCH)/boot/dts/* $(IB_DTSDIR); \
	fi
	[ -d $(KERNEL_BUILD_DIR)/firmware ] && $(CP) $(KERNEL_BUILD_DIR)/firmware $(IB_KDIR)/ || true # and optional firmware
	(cd $(LINUX_DIR) ; find . -name Makefile -o -name Kconfig -o -name \*.pl > $(IB_KDIR)/.headers.lst)
	(cd $(LINUX_DIR) ; find Module.symvers scripts -type f >> $(IB_KDIR)/.headers.lst)
	(cd $(LINUX_DIR) ; find arch/x86/include include -type f >> $(IB_KDIR)/.headers.lst)
	(cd $(LINUX_DIR) ; tar -c -f - -T $(IB_KDIR)/.headers.lst ) | (cd $(IB_LDIR); tar -xf -)
	$(SED) 's,^# REVISION:=.*,REVISION:=$(REVISION),g' $(PKG_BUILD_DIR)/include/version.mk
	find $(PKG_BUILD_DIR) -name CVS -o -name .git -o -name .svn \
	  | $(XARGS) rm -rf
	find $(STAGING_DIR_HOST)/bin -maxdepth 1 -type f -perm -u=x \
	  | $(XARGS) $(SCRIPT_DIR)/bundle-libraries.sh $(PKG_BUILD_DIR)/staging_dir/host/bin
	if [ -d $(TOPDIR)/files ]; then \
		rm -rf $(PKG_BUILD_DIR)/files ; \
		mkdir -p $(PKG_BUILD_DIR)/files ; \
		rsync -qax $(TOPDIR)/files/ $(PKG_BUILD_DIR)/files/ ; \
	fi
	mkdir -p $(PKG_BUILD_DIR)/staging_dir/host/lib
	$(CP) $(STAGING_DIR_HOST)/lib/grub $(PKG_BUILD_DIR)/staging_dir/host/lib
	$(MAKE) -C $(PKG_BUILD_DIR) package_index
	$(TAR) c -C $(BUILD_DIR) $(IB_NAME) | bzip2 -c > $@

download:
prepare:
compile: $(BIN_DIR)/$(IB_NAME).tar.bz2
install: compile

clean: FORCE
	rm -rf $(PKG_BUILD_DIR) $(BIN_DIR)/$(IB_NAME).tar.bz2
