
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mce-inject
PKG_VERSION:=2013-01-19
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://git.kernel.org/pub/scm/utils/cpu/mce/mce-inject.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=4cbe46321b4a81365ff3aafafe63967264dbfec5
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Andi Kleen <ak@linux.intel.com>

include $(INCLUDE_DIR)/package.mk

define Package/mce-inject
  SECTION:=opt
  CATEGORY:=Utilities
  TITLE:=MCE Injection tool for Linux
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) 
endef

define Package/mce-inject/description
  mce-inject injects synthetic Machine Check Exceptions into the CPU for testing.
endef

define Package/mce-inject/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mce-inject $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mce-inject))
