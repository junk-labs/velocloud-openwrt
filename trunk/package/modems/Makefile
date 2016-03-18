#
# Copyright (C) 2007-2014 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=modems
PKG_RELEASE:=1
PKG_VERSION:=1.0-1
PKG_LICENSE:=GPL-2.0

#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/modems
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Modem support for OpenWrt
  DEPENDS:=+comgt +libqmi
  URL:=http://openwrt.org/
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef

define Package/modems/description
 This package contains a minimal modem subsystem for OpenWrt
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default

endef

define Build/InstallDev

endef

Build/Compile = $(Build/Compile/Default)

define Package/modems/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,modems))
