# Copyright (C) 2014 Velocloud Inc
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cdrkit
PKG_VERSION:=1.1.11
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://archive.ubuntu.com/ubuntu/pool/main/c/cdrkit/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_MD5SUM:=efe08e2f3ca478486037b053acd512e9

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/cdrkit
  SECTION:=base
  CATEGORY:=VeloCloud
  VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
  TITLE:=Highly portable CD/DVD/BluRay command line recording software
  URL:=http://cdrtools.sourceforge.net/private/cdrkit.html
  DEPENDS:= +zlib +libbz2 +libmagic
endef

define Package/cdrkit/description
  Highly portable CD/DVD/BluRay command line recording software.
endef

CONFIGURE_VARS += 
MAKE_FLAGS += 
CONFIGURE_ARGS = \
	--prefix=$(CONFIGURE_PREFIX)

define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/cdrkit/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/genisoimage/genisoimage $(1)/usr/sbin/genisoimage
	$(LN) genisoimage $(1)/usr/sbin/mkisofs
endef

$(eval $(call BuildPackage,cdrkit))
