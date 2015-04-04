# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=makedumpfile
PKG_VERSION:=1.5.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://sourceforge.net/projects/makedumpfile/files/makedumpfile/1.5.7/
PKG_MD5SUM:=31668a6dfb8823dd0b7ac09d06fb902e
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libelf elfutils bzip2


include $(INCLUDE_DIR)/package.mk

define Package/makedumpfile
  MAINTAINER:= jordan@velocloud.net
  SECTION:=base
  CATEGORY:=VeloCloud
  TITLE:=makedumpfile
  DEPENDS:=libelf1 +libdw +libbz2
endef

define Package/makedumpfile/description
  makedumpfile is kernel core dump utility to generate kernel cores.
endef

# Replace a bunch of defines from the Makefile to add our flags,
# since the Makefile itself provides no nice way of doing this.
# Their "CROSS" hack is totally broken for real cross compiles.
DATE=18 Sep 2014

CROSS_CFLAGS = $(TARGET_CFLAGS) \
	-g -O2 -Wall -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
	-DVERSION='\"$(PKG_VERSION)\"' -DRELEASE_DATE='\"$(DATE)\"'
CROSS_CFLAGS_ARCH     = $(TARGET_CFLAGS) \
	-g -O2 -Wall -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(CROSS_CFLAGS)" \
		CFLAGS_ARCH="$(CROSS_CFLAGS_ARCH)" \
		LINKTYPE="dynamic"
endef

define Package/makedumpfile/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/makedumpfile $(1)/sbin/
endef

$(eval $(call BuildPackage,makedumpfile))
