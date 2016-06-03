#
# Copyright (C) 2015 VeloCloud Networks
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pam_tacplus
PKG_VERSION:=1.3.9-20160527
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/jeroennijhof/pam_tacplus
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION=7c1b1224ccf33ea87cc30181cb15aa26c1c24044
PKG_MAINTAINER:=Shankar Unni <shankar@velocloud.net>

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/pam_tacplus
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpam +libopenssl
  TITLE:=TACACS+ PAM plugin
  URL:=https://github.com/jeroennijhof/pam_tacplus
endef

define Package/pam_tacplus/description
	TACACS+ PAM Plugin
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--exec-prefix=/ \
		--disable-doc \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/*.so* $(1)/lib/
endef

define Package/pam_tacplus/install
	$(INSTALL_DIR) $(1)/lib $(1)/lib/security
	$(INSTALL_DIR) $(1)/usr $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/lib/*.so* $(1)/lib/
	$(CP) $(PKG_INSTALL_DIR)/lib/security/*.so* $(1)/lib/security/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pam_tacplus))
