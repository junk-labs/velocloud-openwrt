#
# Copyright (C) 2006-2015 OpenWrt.org (VeloCloud Networks, Inc.)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

VC_MENU:=VeloCloud support

define KernelPackage/vc-nmi
  SUBMENU:=$(VC_MENU)
#  CATEGORY:=VeloCloud
  TITLE:=VeloCloud NMI handler
  DEPENDS:=@TARGET_x64_edge5x0
  DEFAULT:=y if (TARGET_x64_edge5x0)
  # Note: force this to be statically linked in.
  KCONFIG:= \
	CONFIG_VELOCLOUD_NMI=y
  #FILES:=$(LINUX_DIR)/drivers/platform/x86/velocloud-nmi.ko
  #AUTOLOAD:=$(call AutoLoad,06,velocloud-nmi)
endef

define KernelPackage/vc-nmi/description
 Kernel module for VeloCloud NMI handling.
endef

$(eval $(call KernelPackage,vc-nmi))
