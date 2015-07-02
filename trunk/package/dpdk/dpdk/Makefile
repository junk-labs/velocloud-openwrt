#
# Copyright (C) 2015 VeloCloud Networks, Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dpdk
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dpdk.org/browse/dpdk/snapshot/
PKG_MD5SUM:=e9e7935c9eec920841ad373949514934

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/dpdk/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=@i386||x86_64 +libpthread +librt +kmod-hwmon-core
  URL:=http://dpdk.org/
endef

define Package/dpdk
$(call Package/dpdk/Default)
  TITLE:=Intel Data Plane Development Kit
endef

define Package/dpdk/description
Intel DPDK (Data Plane Development Kit).
endef

TARCH=$(patsubst i386,i686,$(ARCH))
T=$(TARCH)-openwrt-linuxapp-gcc
DPDK_DEBUG=-g

define Build/Compile
	echo FOO Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" install \
		T=$(T) V=$(V) \
		CONFIG_RTE_BUILD_COMBINE_LIBS=y \
		CROSS="$(TARGET_CROSS)" \
		OPENWRT_CFLAGS="$(TARGET_CFLAGS) $(DPDK_DEBUG) -msse3 -msse4" \
		OPENWRT_LDFLAGS="$(TARGET_LDFLAGS) $(DPDK_DEBUG) " \
		OPENWRT_LDLIBS="$(TARGET_LDLIBS)" \
		RTE_KERNELDIR="$(LINUX_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	echo FOO Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/dpdk/include \
		$(1)/usr/dpdk/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/include/* \
		$(1)/usr/dpdk/include
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/lib/libintel_dpdk.a \
		$(1)/usr/dpdk/lib
endef
define Package/dpdk/install
	echo FOO Package/dpdk/install
	$(INSTALL_DIR) \
		$(1)/usr/share/dpdk/app \
		$(1)/usr/share/dpdk/kmod
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/app/* \
		$(1)/usr/share/dpdk/app
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/kmod/* \
		$(1)/usr/share/dpdk/kmod
endef

$(eval $(call BuildPackage,dpdk))
