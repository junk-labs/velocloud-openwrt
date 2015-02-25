# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=makedumpfile
PKG_VERSION:=1.5.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://sourceforge.net/projects/makedumpfile/files/makedumpfile/1.5.7/
PKG_MD5SUM:=31668a6dfb8823dd0b7ac09d06fb902e
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libelf


include $(INCLUDE_DIR)/package.mk

define Package/makedumpfile
  MAINTAINER:= jordan@velocloud.net
  SECTION:=base
  CATEGORY:=VeloCloud
  TITLE:=makedumpfile
  DEPENDS:=+libelf
endef

$(CROSS)

define Package/makedumpfile/description
  makedumpfile is kernel core dump utility to generate kernel cores.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) 
endef

define Package/makedumpfile/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/makedumpfile $(1)/sbin/
endef

$(eval $(call BuildPackage,makedumpfile))
