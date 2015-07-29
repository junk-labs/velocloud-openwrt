#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=traceroute
PKG_VERSION:=2.0.21
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/traceroute
PKG_MD5SUM:=79289adabd6f3ebf9160fc0815ab3150

PKG_MAINTAINER:=Velocloud <support@velocloud.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define Package/traceroute
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Classic Traceroute tool
  URL:=http://traceroute.sourceforge.net/
endef

define Package/traceroute/description
 traceroute classic
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CPPFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE" \
		LDFLAGS="-L../libsupp"
endef

define Package/traceroute/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/traceroute/traceroute $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,traceroute))
