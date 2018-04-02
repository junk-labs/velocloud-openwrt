# Copyright (C) [2011-2012] [TheClashingRocks.org]
#
# Author: Jiva Nath Bagale <jnbagale@gmail.com>
#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# 
include $(TOPDIR)/rules.mk

PKG_NAME:=sysbench
PKG_VERSION:=0.4.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/sysbench
PKG_MD5SUM:=3a6d54fdd3fe002328e4458206392b9d
PKG_MAINTAINER:=Dhruva <dhruva@velocloud.net>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=zcat
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/sysbench
    TITLE:=Sysbench
    SECTION:=utils
    DEPENDS:=+libpthread +librt
    CATEGORY:=Utilities
    URL:=http://www.rt-embedded.com/sysbench
endef

define Package/sysbench/description
  PCD – Process Control Daemon is a light-weight system level
  process manager, designed for Embedded Linux based products.
  The PCD is used for starting the system in a synchronized
  manner, control and monitor processes and recover from
  crashes.
endef


TARGET_CFLAGS += $(FPIC()
CONFIGURE_ARGS += \
		  --without-mysql


define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef


define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Package/sysbench/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/sysbench $(1)/usr/bin
endef

$(eval $(call BuildPackage,sysbench))
