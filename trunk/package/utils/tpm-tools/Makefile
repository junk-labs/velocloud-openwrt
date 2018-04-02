#
# Copyright (C) 2007-2015 Velocloud Inc
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tpm-tools
PKG_VERSION:=1.3.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://sourceforge.net/projects/trousers/files/tpm-tools/1.3.8
PKG_MD5SUM:=85a978c4e03fefd4b73cbeadde7c4d0b
PKG_MAINTAINER := none

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tpm-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=TPM Tools
  URL:=http://www.linux-usb.org/
  DEPENDS:=+librt +libpthread +trousers
endef

define Build/Prepare
	$(Build/Prepare/Default)
endef

define Package/tpm-tools/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtpm_unseal.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/tpm_* $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,tpm-tools))
