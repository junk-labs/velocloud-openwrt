#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lualogging
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Neopallium/lualogging
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=cadc4e8fd652be07a65b121a3e024838db330c15
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/lualogging
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaLogging
  URL:=https://github.com/Neopallium/lualogging
  # forget about the dependencies for sql, etc.
  DEPENDS:=+lua
endef

define Package/lualogging/description
  LuaLogging is a log4j-like logging library in lua.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/lualogging/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/src/logging* $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lualogging))
