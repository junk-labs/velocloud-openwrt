#
# Copyright (c) 2012 velocloud.net
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mvswconf
PKG_REV:=1.0
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=mvswconf-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_CAT:=rsync -av $(CURDIR)/src/
#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mvswconf
  MAINTAINER:= shankar@velocloud.net
  SECTION:=net
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=mvswconf
  URL:=http://velocloud.net
  DEPENDS:=+jansson
endef

MAKE_FLAGS += \
	COPTS="$(TARGET_CFLAGS)" \

TARGET_CFLAGS += -g
TARGET_LDFLAGS += -ljansson

define Package/mvswconf/description
 A tool to program Marvell switches in VeloCloud Edges
endef

#define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
#	$(CP) ./src $(PKG_BUILD_DIR)
#endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/mvswconf/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mvswconf $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mvswconf))
