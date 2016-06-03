#
# Copyright (C) 2015 VeloCloud Networks
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nss_tacplus
PKG_VERSION:=2015-12-22
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/benschumacher/nss_tacplus
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION=e03def49c2ebd1c5f5ed40d7b35faea24054aba5
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MAINTAINER:=Shankar Unni <shankar@velocloud.net>

PKG_INSTALL:=1
# PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/nss_tacplus
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+pam_tacplus
  TITLE:=TACACS+ NSS plugin
  URL:=https://github.com/benschumacher/nss_tacplus
endef

define Package/nss_tacplus/description
	TACACS+ NSS Plugin
endef

TARGET_CFLAGS += -fPIC -std=gnu99 -D_FORTIFY_SOURCE=2 -fstack-protector
MAKE_FLAGS += $(MAKE_VARS)

define Package/nss_tacplus/install
	$(INSTALL_DIR) $(1)/lib 
	$(CP) $(PKG_INSTALL_DIR)/lib/*.so* $(1)/lib/
endef

$(eval $(call BuildPackage,nss_tacplus))
