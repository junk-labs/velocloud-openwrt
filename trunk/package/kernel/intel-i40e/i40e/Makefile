#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=intel-i40e
PKG_VERSION:=2.4.3
PKG_RELEASE:=1

PKG_SOURCE=i40e-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=@SF/e1000/i40e%20stable
#PKG_MD5SUM:=071cf7fa263d3c202a612fd6858b6822

include $(INCLUDE_DIR)/package.mk


define KernelPackage/intel-i40e
  SUBMENU:=Network Devices
  DEPENDS:=@PCIE_SUPPORT +kmod-i2c-algo-bit +kmod-ptp +kmod-mdio kmod-hwmon-core
  KCONFIG:=CONFIG_I40E
  TITLE:=Intel External Driver for i40e
  FILES:=$(PKG_BUILD_DIR)/src/i40e.ko
  AUTOLOAD:=$(call AutoLoad,36,i40e)
endef

define KernelPackage/intel-i40e/description
 Intel-published Portable I40E Driver
endef

TAR_CMD=$(HOST_TAR) -C $(1) --strip-components=1 $(TAR_OPTIONS)
MAKE_PATH=./src
MAKE_FLAGS += KSRC="$(LINUX_DIR)"

$(eval $(call KernelPackage,intel-i40e))
