#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntpstat
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://anonscm.debian.org/git/openstack/third-party/ntpstat.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_VERSION:=4793c66f6111057c99f1f45fada65b753cf97e83

include $(INCLUDE_DIR)/package.mk

define Package/ntpstat
  SUBMENU:=Time Synchronization Status
  SECTION:=net
  CATEGORY:=Network
  TITLE:=NTP (Network Time Protocol) status tool
  URL:=https://packages.debian.org/source/sid/ntpstat
endef

define Package/ntpstat/description
	Tool to display NTP synchronization status
endef

define Package/ntpstat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpstat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ntpstat))
