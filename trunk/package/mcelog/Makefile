
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mcelog
PKG_VERSION:=2016-02-19
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://git.kernel.org/pub/scm/utils/cpu/mce/mcelog.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=b4ec9cb2b4e0dd6246d663bff0b7483dd96e8c10
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Andi Kleen <ak@linux.intel.com>

include $(INCLUDE_DIR)/package.mk

define Package/mcelog
  SECTION:=opt
  CATEGORY:=Utilities
  TITLE:=the linux hardware error daemon
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) 
endef

define Package/mcelog/description
  mcelog processes machine checks (in particular memory and CPU hardware errors) on modern x86 Linux systems.
endef

define Package/mcelog/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/mcelog.init $(1)/etc/init.d/mcelog
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mcelog $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mcelog.conf $(1)/etc
endef

$(eval $(call BuildPackage,mcelog))
