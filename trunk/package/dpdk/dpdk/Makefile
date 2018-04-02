#
# Copyright (C) 2015 VeloCloud Networks, Inc.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dpdk
PKG_VERSION:=16.07
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dpdk.org/browse/dpdk/snapshot/
PKG_MD5SUM:=4afdc7951e21ff878a85ecade7f6f488

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dpdk/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=@i386||x86_64 +libpcap +libpthread +librt +kmod-hwmon-core
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
	$(INSTALL_DIR) \
		$(1)/usr/dpdk/include \
		$(1)/usr/dpdk/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/include/dpdk/* \
		$(1)/usr/dpdk/include
	$(CP) \
		$(PKG_INSTALL_DIR)/lib/*.a \
		$(1)/usr/dpdk/lib
	$(LN) \
		libdpdk.a \
		$(1)/usr/dpdk/lib/libintel_dpdk.a
endef
define Package/dpdk/install
	$(INSTALL_DIR) \
		$(1)/usr/share/dpdk/bin \
		$(1)/usr/share/dpdk/kmod
	$(INSTALL_BIN) -s \
		$(PKG_INSTALL_DIR)/bin/* \
		$(1)/usr/share/dpdk/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/lib/modules/*/extra/dpdk/*.ko \
		$(1)/usr/share/dpdk/kmod
endef

$(eval $(call BuildPackage,dpdk))
