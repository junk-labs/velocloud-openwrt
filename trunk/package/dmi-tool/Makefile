#
# Copyright (c) 2012 velocloud.net
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dmi-tool
PKG_REV:=1
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=dmi-tool-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_CAT:=rsync -av $(CURDIR)/src/
PKG_MAINTAINER:=Sandra Berndt <sberndt@velocloud.net>

include $(INCLUDE_DIR)/package.mk

define Package/dmi-tool
  SECTION:=utils
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=Tool to program DMI info
  URL:=http://velocloud.net
  DEPENDS:=+USE_EGLIBC:libbsd +libuuid
endef

define Package/dmi-tool/config
endef

define Package/dmi-tool/description
 Tool to program DMI info
endef

define Package/dmi-tool/conffiles
endef

MAKE_FLAGS += \
	COPTS="$(TARGET_CFLAGS)" \
	STATIC_WORKER="fork"

TARGET_CFLAGS += -Dlint -DHAVE_CONFIG_H -I. 

ifneq ($(CONFIG_USE_EGLIBC),)
  TARGET_LDFLAGS += -lbsd -luuid
endif

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/dmi-tool/install
	$(INSTALL_DIR) $(1)/root/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dmi-tool $(1)/root/
endef

$(eval $(call BuildPackage,dmi-tool))
