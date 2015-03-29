#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=x86_64
BOARD:=x64
BOARDNAME:=x64
FEATURES:=squashfs ext4 vdi vmdk pcmcia targz
SUBTARGETS=generic edge500 edge5x0 edge1000 pw-dev vc-vmdk vc-xen-aws vc-kvm-guest

KERNEL_PATCHVER:=3.14

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_TARGET_ROOTFS_ISO),RequireCommand,Ignore),mkisofs, \
   	Please install mkisofs. \
))
