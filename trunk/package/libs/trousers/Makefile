#
# Copyright (C) 2015 Velocloud Inc
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=trousers
PKG_VERSION:=0.3.13
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/trousers
PKG_MD5SUM:=ad508f97b406f6e48cd90e85d78e7ca8
PKG_MAINTAINER:=none

PKG_LICENSE:=LGPLv2
PKG_LICENSE_FILES:=COPYING

#PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

define Package/trousers
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=TCG Trousers libs
  URL:=http://trousers.sourceforge.net
  DEPENDS+= +libopenssl +libcrypto
endef

define Package/trousers/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += \
	$(FPIC) \
	-DVC_INST \

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \

define Build/Compile
	$(call Build/Compile/Default, \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		all
	+$(MAKE) -i $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/trousers
	$(CP) $(PKG_INSTALL_DIR)/usr/include/trousers/*.h $(1)/usr/include/trousers/
	$(INSTALL_DIR) $(1)/usr/include/tss
	$(CP) $(PKG_INSTALL_DIR)/usr/include/tss/*.h $(1)/usr/include/tss/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtspi.{a,so*} $(1)/usr/lib/
endef

define Package/trousers/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtspi.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/tcsd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,trousers))
