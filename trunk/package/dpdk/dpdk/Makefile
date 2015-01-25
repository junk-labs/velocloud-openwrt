#
# Copyright (C) 2015 VeloCloud Networks, Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dpdk
PKG_VERSION:=1.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dpdk.org/browse/dpdk/snapshot/
PKG_MD5SUM:=11ad8785aaa869cc87265bcb8d828f22

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

define Build/Compile
	echo FOO Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" install \
		T=$(T) V=$(V) \
		CROSS="$(TARGET_CROSS)" \
		OPENWRT_CFLAGS="$(TARGET_CFLAGS) -msse3 -msse4" \
		OPENWRT_LDFLAGS="$(TARGET_LDFLAGS)" \
		OPENWRT_LDLIBS="$(TARGET_LDLIBS)" \
		RTE_KERNELDIR="$(LINUX_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	echo FOO Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/include/dpdk \
		$(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/include/* \
		$(1)/usr/include/dpdk
	$(CP) \
		$(PKG_INSTALL_DIR)/$(T)/lib/* \
		$(1)/usr/lib
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
