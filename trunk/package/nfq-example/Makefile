#
# Copyright (c) 2012 velocloud.net
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nfq-example
PKG_REV:=1
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=nfq-example-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_CAT:=rsync -av $(CURDIR)/src/
PKG_MAINTAINER:=Sandra Berndt <sberndt@velocloud.net>

include $(INCLUDE_DIR)/package.mk

define Package/nfq-example
  SECTION:=net
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=libnetfilter-queue Example
  URL:=http://velocloud.net
  DEPENDS:=+USE_EGLIBC:libbsd +libnetfilter-queue
endef

define Package/nfq-example/config
	source "$(SOURCE)/Config.in"
endef

define Package/nfq-example/description
 This is an example for how to use libnetfilter-queue for user space packet processing.
endef

define Package/nfq-example/conffiles
endef

MAKE_FLAGS += \
	COPTS="$(TARGET_CFLAGS)" \
	STATIC_WORKER="fork"

TARGET_CFLAGS += -Dlint -DHAVE_CONFIG_H -I. 

ifneq ($(CONFIG_USE_EGLIBC),)
  TARGET_LDFLAGS += -lbsd
endif
TARGET_LDFLAGS += -lnetfilter_queue

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/nfq-example/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nfq-example $(1)/usr/bin/
endef

$(eval $(call BuildPackage,nfq-example))
