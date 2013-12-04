#
# Copyright (c) 2012 velocloud.net
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vcsystests
PKG_REV:=1
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=vcsystests-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_CAT:=rsync -av $(CURDIR)/src/
#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vcsystests
  MAINTAINER:= dhruva@velocloud.net
  SECTION:=net
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=vcsystests
  URL:=http://velocloud.net
  DEPENDS:=+libcunit +libpthread +libncurses +libpcap
endef

MAKE_FLAGS += \
	COPTS="$(TARGET_CFLAGS)" \

TARGET_CFLAGS += -g -O0 -I. -Iinclude
TARGET_LDFLAGS += -lcunit -lpcap -lcurses

define Package/vcsystests/description
 This is an example for how to use libnetfilter-queue for user space packet processing.
endef


#define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
#	$(CP) ./src $(PKG_BUILD_DIR)
#endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/vcsystests/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vcsystests $(1)/usr/bin/
endef

$(eval $(call BuildPackage,vcsystests))
