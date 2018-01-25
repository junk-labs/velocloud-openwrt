#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lshw
PKG_VERSION:=B.02.18
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.ezix.org/software/files/

include $(INCLUDE_DIR)/package.mk

define Package/lshw
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=disc
  TITLE:=Hardware Inventory Lister
  DEPENDS=+libstdcpp
  URL:=http://lshw.ezix.org
endef

define Package/lshw/description
    Hardware Inventory Lister
endef

#define Build/Configure
#	$(call Build/Configure/Default, --with-readline --disable-device-mapper)
#endef

define Package/lshw/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/lshw $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,lshw))
