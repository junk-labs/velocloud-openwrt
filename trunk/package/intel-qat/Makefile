#
# Copyright (C) 2015 velocloud.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=intel-qat
PKG_REV:=2.6.0-60
PKG_QAT:=1.5
PKG_QAT_REV:=1.11.0-36
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_TAR:=qatmux.l.$(PKG_REV).tar
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=intel-qat-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/build-$(PKG_QAT)
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Sandra Berndt <sberndt@velocloud.net>

include $(INCLUDE_DIR)/package.mk

ADF_DEBUG:=

# user land

# Do not make intel-qat depend on kmod-intel-qat.
# We only install the kmod on systems that have a QAT module, while
# the intel-qat package (library, etc.) go on all systems.
define Package/intel-qat
  SECTION:=utils
  CATEGORY:=VeloCloud
  TITLE:=Intel Quickassist user libs and tools.
  URL:=http://velocloud.net
  DEPENDS:=+USE_EGLIBC +zlib +libopenssl
endef

define Package/intel-qat/description
  Intel Quickassist user libs and tools.
endef

# kernel land;

define KernelPackage/intel-qat
  SUBMENU:=Other modules
  TITLE:=Intel Quickassist Kernel Driver
  URL:=https://01.org/packet-processing/intel®-quickassist-technology-drivers-and-patches
  MAINTAINER:=none
  DEPENDS+= @PCI_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/icp_qa_al.ko $(PKG_BUILD_DIR)/qaeMemDrv.ko
  AUTOLOAD:=$(call AutoProbe,intel-qat)
endef

define KernelPackage/intel-qat/description
  This module adds support for Intel Quickassist Technology.
endef

# remove thirdparty and link against our libs;

Build/Prepare/Patches := \
	001-build-march.patch \
	002-build-perf.patch \
	003-sha-init.patch \
	004-config-etc-qat.patch \

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(TAR) -C $(PKG_BUILD_DIR)/.. -xf src/$(PKG_SOURCE_TAR)
	$(TAR) -C $(PKG_BUILD_DIR) -xf $(PKG_BUILD_DIR)/../QAT$(PKG_QAT)/QAT$(PKG_QAT).L.$(PKG_QAT_REV).tar.gz
	$(RM) -rf $(PKG_BUILD_DIR)/quickassist/utilities/osal/thirdparty/
	for patch in $(Build/Prepare/Patches); do \
		echo "Applying patch: $$$${patch}"; \
		$(PATCH) -d $(PKG_BUILD_DIR) -p1 < patches/QAT$(PKG_QAT)/$$$${patch}; \
	done
endef

ifneq ($(ADF_DEBUG),)
ADF_OPTS += \
	ADF_PLATFORM_DEBUG=1 \
	ADF_DRIVERS_DEBUG=1 \
	ADF_CONFIG_DEBUG=1 \
	ADF_ACCEL_ENG_DEBUG=1 \
	ADF_ACCEL_MGR_DEBUG=1
endif

MAKE_OPTS:= \
	$(TARGET_CONFIGURE_OPTS) \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	MACHINE="$(LINUX_KARCH)" \
	KERNEL_SOURCE_ROOT="$(LINUX_DIR)" \
	LIB_EXTRA_PATH="$(TARGET_LDFLAGS)" \
	ICP_ROOT="$(PKG_BUILD_DIR)" \
	ICP_ENV_DIR="$(PKG_BUILD_DIR)/quickassist/build_system/build_files/env_files" \
	ICP_ARCH_USER="$(LINUX_KARCH)" \
	ICP_BUILDSYSTEM_PATH="$(PKG_BUILD_DIR)/quickassist/build_system" \
	ICP_BUILD_OUTPUT="$(PKG_BUILD_DIR)" \
	ICP_TOOLS_TARGET=accelcomp \
	$(ADF_OPTS) \

define Build/Compile/kmod-intel-qat
	$(MAKE) -C "$(PKG_BUILD_DIR)/quickassist" \
		$(MAKE_OPTS) \
		kernel
endef

define Build/Compile/intel-qat
	$(MAKE) -C "$(PKG_BUILD_DIR)/quickassist" \
		$(MAKE_OPTS) \
		cmd_line_cflags="$(TARGET_CFLAGS) -g" \
		user
endef

define Build/Compile/intel-qat-perf
	$(MAKE) -C "$(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/src/sample_code" \
		$(MAKE_OPTS) \
		perf_all
	$(CP) $(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/src/sample_code/build/qaeMemDrv.ko $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(call Build/Compile/kmod-intel-qat)
	$(call Build/Compile/intel-qat)
	$(call Build/Compile/intel-qat-perf)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/quickassist/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/access_layer
	$(CP) $(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/include/* $(1)/usr/include/access_layer/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib*.a $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef

define Package/intel-qat/install
	$(INSTALL_DIR) $(1)/bin/ $(1)/lib/ $(1)/etc/qat
	$(INSTALL_BIN) -s $(PKG_BUILD_DIR)/adf_ctl $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libicp_qa_al_s.so $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quickassist/lookaside/access_layer/src/sample_code/build/cpa_sample_code $(1)/bin/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/quickassist/config/c2xxx_qa_dev0_single_ae.conf $(1)/etc/qat/c2xxx_qa_dev0.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/quickassist/config/dh89xxcc_qa_dev0_single_accel.conf $(1)/etc/qat/dh89xxcc_qa_dev0.conf
endef

define KernelPackage/intel-qat/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mmp_firmware.bin $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mmp_firmware_c2xxx.bin $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mof_firmware.bin $(1)/lib/firmware/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mof_firmware_c2xxx.bin $(1)/lib/firmware/
endef

$(eval $(call BuildPackage,intel-qat))
$(eval $(call KernelPackage,intel-qat))

