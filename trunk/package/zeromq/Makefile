# Copyright (C) [2011-2012] [TheClashingRocks.org]
#
# Author: Jiva Nath Bagale <jnbagale@gmail.com>
#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# 
include $(TOPDIR)/rules.mk

PKG_NAME:=zeromq
PKG_VERSION:=4.0.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.zeromq.org/
PKG_MD5SUM=f3c3defbb5ef6cc000ca65e529fdab3b

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/zeromq
    MAINTAINER:=jnbagale@gmail.com
    TITLE:=zeromq
    SECTION:=libs
    DEPENDS:=+libuuid +libstdcpp +libpthread +librt
    CATEGORY:=Libraries
    URL:=http://www.zeromq.org/
endef

define Package/zeromq/description
    The library provides light weight messaging services using an intelligent transport layer. 
    It can carry messages across inproc, IPC, TCP and multicast and is faster than TCP for 
    clustered products and supercomputing.
endef

TARGET_CFLAGS += -O2 -g

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib 	\
		$(1)/usr/include \
		$(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig \
		$(1)/usr/lib/

endef

define Package/zeromq/install
	$(INSTALL_DIR) \
		$(1)/usr/lib 
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/

endef

$(eval $(call BuildPackage,zeromq))
