#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk


PKG_NAME:=iperf
PKG_VERSION:=3.0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://stats.es.net/software/
PKG_MD5SUM:=1bb10c8144460f1b7ce9a4bba55d8dcd
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

#CONFIGURE_ARGS= \
		 --cc=${TARGET_CC} \
		 --extra-cflags="${TARGET_CFLAGS}" \

CONFIG_TARGET_OPTIMIZATION="-O2 -pipe -mtune=generic"

define Package/iperf3
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=network performance test tool
  DEPENDS:=+librt +libpthread +libuuid
  URL:=http://stats.es.net/software
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ CFLAGS=$(CONFIG_TARGET_OPTIMIZATION)
endef

define Package/iperf3/description
    iperf3 is a tool for measuring achievable bandwidth on IP networks.
endef

define Package/iperf3/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/iperf3 $(1)/usr/bin
endef

$(eval $(call BuildPackage,iperf3))

