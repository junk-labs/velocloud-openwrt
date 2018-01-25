#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=intel-i40evf
PKG_VERSION:=3.4.2
PKG_RELEASE:=1

PKG_SOURCE=i40evf-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=@SF/e1000/i40evf%20stable
#PKG_MD5SUM:=b0adf9158ea25c9ac8b334c9b4f1a0e1

include $(INCLUDE_DIR)/package.mk


define KernelPackage/intel-i40evf
  SUBMENU:=Network Devices
  DEPENDS:=@PCIE_SUPPORT +kmod-i2c-algo-bit +kmod-ptp +kmod-mdio
  KCONFIG:=CONFIG_I40EVF
  TITLE:=Intel External Driver for i40e virtual functions
  FILES:=$(PKG_BUILD_DIR)/src/i40evf.ko
  AUTOLOAD:=$(call AutoLoad,37,i40evf)
endef

define KernelPackage/intel-i40evf/description
 Intel-published Portable I40EVF Driver
endef

TAR_CMD=$(HOST_TAR) -C $(1) --strip-components=1 $(TAR_OPTIONS)
MAKE_PATH=./src
MAKE_FLAGS += KSRC="$(LINUX_DIR)"

$(eval $(call KernelPackage,intel-i40evf))
