#
# Copyright (C) 2014 Velocloud Inc.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gptfdisk
PKG_VERSION:=0.8.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/gptfdisk
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=disc
  TITLE:=gptfdisk GPT disk tool
  DEPENDS=+libncurses +libuuid +libblkid +libpopt +libstdcpp
  URL:=http://http://www.rodsbooks.com/gdisk/
endef

define Package/gptfdisk/description
    gptfdisk GPT disk tool
endef

define Build/Configure
	sed -i -e 's#ncursesw#ncurses#'  $(PKG_BUILD_DIR)/Makefile
endef

define Package/gptfdisk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gdisk $(1)/usr/sbin/gdisk
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sgdisk $(1)/usr/sbin/sgdisk
endef

$(eval $(call BuildPackage,gptfdisk))
