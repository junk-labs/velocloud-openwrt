#
# Copyright (c) 2012 velocloud.net
#

include $(TOPDIR)/rules.mk

PKG_NAME:=igb-eeprom
PKG_REV:=1
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=igb-eeprom-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_CAT:=rsync -av $(CURDIR)/src/
PKG_MAINTAINER:=Sandra Berndt <sberndt@velocloud.net>

include $(INCLUDE_DIR)/package.mk

define Package/igb-eeprom
  SECTION:=net
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=Tool to program blank Intel IGB 82580 EEPROM
  URL:=http://velocloud.net
  DEPENDS:=+USE_EGLIBC:libbsd
endef

define Package/igb-eeprom/config
endef

define Package/igb-eeprom/description
 Tool to program blank Intel IGB 82580 EEPROM.
endef

define Package/igb-eeprom/conffiles
endef

MAKE_FLAGS += \
	COPTS="$(TARGET_CFLAGS)" \
	STATIC_WORKER="fork"

TARGET_CFLAGS += -Dlint -DHAVE_CONFIG_H -I. 

ifneq ($(CONFIG_USE_EGLIBC),)
  TARGET_LDFLAGS += -lbsd
endif

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/igb-eeprom/install
	$(INSTALL_DIR) $(1)/root/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/igb-eeprom $(1)/root/
	for file in `ls $(PKG_BUILD_DIR)/intel/*.bin`; do \
		$(INSTALL_BIN) $$$$file $(1)/root/; \
	done
endef

$(eval $(call BuildPackage,igb-eeprom))
