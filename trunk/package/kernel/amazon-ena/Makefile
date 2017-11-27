#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=amazon-ena
PKG_VERSION:=1.3.0
PKG_RELEASE:=1
PKG_REV:=b0eff7b0287db28432c473e76ec557a108567f44

PKG_SOURCE:=amzn-drivers-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL=https://github.com/amzn/amzn-drivers
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk


define KernelPackage/amazon-ena
  SUBMENU:=Network Devices
  TITLE:=External Driver for Amazon Web Services ENA driver
  DEPENDS:=@(PCI_SUPPORT)
  FILES:=$(PKG_BUILD_DIR)/kernel/linux/ena/ena.ko
  AUTOLOAD:=$(call AutoLoad,38,ena)
endef

define KernelPackage/amazon-ena/description
 External Driver for Amazon Web Services ENA driver
endef

CFLAGS+=-DUBUNTU_VERSION_CODE=

MAKE_FLAGS += \
	-C "$(LINUX_DIR)" \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)/kernel/linux/ena"

$(eval $(call KernelPackage,amazon-ena))
