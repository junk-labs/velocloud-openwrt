# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=crash
PKG_VERSION:=7.1.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://people.redhat.com/anderson
PKG_MD5SUM:=9f2584041a665eb8580de1e6c642c594
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=gdb less binutils ncurses


include $(INCLUDE_DIR)/package.mk

define Package/crash
  MAINTAINER:= jordan@velocloud.net
  SECTION:=base
  CATEGORY:=VeloCloud
  TITLE:=crash
  DEPENDS:=+gdb +less +binutils +ncurses
endef

$(CROSS)

define Package/crash/description
  crash is a kernel core debugger with gdb extensions built-in.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) 
endef

define Package/crash/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/crash $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,crash))
