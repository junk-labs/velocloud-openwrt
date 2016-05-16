#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=intel-ixgbe
PKG_VERSION:=4.3.15
PKG_RELEASE:=1

PKG_SOURCE=ixgbe-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=@SF/e1000/ixgbe%20stable

include $(INCLUDE_DIR)/package.mk


define KernelPackage/intel-ixgbe
  SUBMENU:=Network Devices
  DEPENDS:=@PCIE_SUPPORT +kmod-i2c-algo-bit +kmod-ptp +kmod-mdio kmod-hwmon-core
  KCONFIG:=CONFIG_IXGBE \
    CONFIG_IXGBE_HWMON=n \
    CONFIG_IXGBE_DCA=n \
    CONFIG_IXGBE_DCB=n
  TITLE:=Intel External Driver for ixgbe
  FILES:=$(PKG_BUILD_DIR)/src/ixgbe.ko
  AUTOLOAD:=$(call AutoLoad,36,ixgbe)
endef

define KernelPackage/intel-ixgbe/description
 Intel-published Portable IXGBE Driver
endef

TAR_CMD=$(HOST_TAR) -C $(1) --strip-components=1 $(TAR_OPTIONS)
MAKE_PATH=./src
MAKE_FLAGS += KSRC="$(LINUX_DIR)"

$(eval $(call KernelPackage,intel-ixgbe))
