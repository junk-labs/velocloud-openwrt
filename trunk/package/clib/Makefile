#
# Copyright (C) 2015 VeloCloud Networks, Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=clib
PKG_VERSION:=2013-11-09
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL=https://github.com/vnet/clib
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION=94d225a9bf869e4d795be4057be6e8aa7c006b9d

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/clib/Default
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=+libpthread 
  URL:=https://fd.io/
endef

define Package/clib
$(call Package/clib/Default)
  TITLE:=Collection of C libraries From Elliot/Dave Barach
endef

define Package/clib/description
Collection of C libraries From Elliot/Dave Barach
endef

CLIB_DEBUG=-g

define Build/Compile
	echo FOO Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" \
		V=$(V) \
		CROSS="$(TARGET_CROSS)" \
		OPENWRT_CFLAGS="$(TARGET_CFLAGS) $(CLIB_DEBUG) " \
		OPENWRT_LDFLAGS="$(TARGET_LDFLAGS) $(CLIB_DEBUG) " \
		OPENWRT_LDLIBS="$(TARGET_LDLIBS)" \
		DESTDIR="$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/include/clib \
		$(1)/usr/lib
	$(CP) \
		$(PKG_BUILD_DIR)/clib/*.h \
		$(1)/usr/include/clib
	$(CP) \
		$(PKG_BUILD_DIR)/.libs/libclib.a \
		$(1)/usr/lib
endef

define Package/clib/install
endef

$(eval $(call BuildPackage,clib))
