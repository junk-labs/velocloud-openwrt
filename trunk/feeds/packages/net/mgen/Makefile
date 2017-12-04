#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mgen
PKG_VERSION:=5.02c
PKG_RELEASE:=1

PKG_SOURCE:=src-$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://downloads.pf.itd.nrl.navy.mil/mgen/
PKG_MD5SUM:=1b1675ddf6848fd3a6bc2e22b0d4a0b5

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

TAR_CMD:=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components 1 $(TAR_OPTIONS)

define Package/mgen
  TITLE:=Multi-Generator
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread $(CXX_DEPENDS)
  URL:=http://cs.itd.nrl.navy.mil/work/mgen/
endef

define Package/mgen/Description
       The Multi-Generator (MGEN) is open source software developed by
       the Naval Research Laboratory (NRL) PROTocol Engineering Advanced
       Networking (PROTEAN) Research Group. MGEN provides the ability to
       perform IP network performance tests and measurements using UDP/IP
       traffic (TCP is currently being developed).
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/makefiles \
		$(TARGET_CONFIGURE_OPTS) \
		CC="$(TARGET_CXX)" \
		SYSTEM_INCLUDES="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -I../include/unix" \
		SYSTEM_LDFLAGS="$(TARGET_LDFLAGS)" \
		SYSTEM_LIBS="-ldl -lpthread" \
		-f Makefile.linux mgen
endef

define Package/mgen/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/makefiles/mgen $(1)/usr/bin/mgen
endef

$(eval $(call BuildPackage,mgen))
