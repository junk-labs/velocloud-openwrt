#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=intel-ixgbevf
PKG_VERSION:=3.1.2
PKG_RELEASE:=1

PKG_SOURCE=ixgbevf-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=@SF/e1000/ixgbevf%20stable

include $(INCLUDE_DIR)/package.mk


define KernelPackage/intel-ixgbevf
  SUBMENU:=Network Devices
  DEPENDS:=@PCIE_SUPPORT +kmod-i2c-algo-bit +kmod-ptp +kmod-mdio
  KCONFIG:=CONFIG_IXGBEVF
  TITLE:=Intel External Driver for ixgbe virtual functions
  FILES:=$(PKG_BUILD_DIR)/src/ixgbevf.ko
  AUTOLOAD:=$(call AutoLoad,37,ixgbevf)
endef

define KernelPackage/intel-ixgbevf/description
 Intel-published Portable IXGBEVF Driver
endef

TAR_CMD=$(HOST_TAR) -C $(1) --strip-components=1 $(TAR_OPTIONS)
MAKE_PATH=./src
MAKE_FLAGS += KSRC="$(LINUX_DIR)"

$(eval $(call KernelPackage,intel-ixgbevf))
