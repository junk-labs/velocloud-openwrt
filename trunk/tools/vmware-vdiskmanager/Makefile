# Copyright (C) 2016 velocloud.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vmware-vdiskmanager
PKG_REV:=5.5.4-2454786
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=$(PKG_REV)

PKG_SOURCE_PROTO:=none
PKG_SOURCE_TAR:=VMware-vix-disklib-$(PKG_REV).x86_64.tar.gz
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)

#PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/build-$(PKG_QAT)
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Shankar Unni <shankar@velocloud.net>

include $(INCLUDE_DIR)/host-build.mk

HOST_UNPACK:=tar -C $(HOST_BUILD_DIR) -xf ./src/$(PKG_SOURCE_TAR)
HOST_SRC_BASE=$(HOST_BUILD_DIR)/vmware-vix-disklib-distrib

define Host/Compile
	echo No build
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_SRC_BASE)/bin64/vmware-vdiskmanager $(STAGING_DIR_HOST)/bin/
	# these two have to go in .../bin. Stupid vmware..
	$(INSTALL_BIN) $(HOST_SRC_BASE)/lib64/libcrypto.so.0.9.8 $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_SRC_BASE)/lib64/libssl.so.0.9.8 $(STAGING_DIR_HOST)/bin/
	# these go into .../lib
	$(CP) $(HOST_SRC_BASE)/lib64/libvixDiskLib.so* $(STAGING_DIR_HOST)/lib/
	$(CP) $(HOST_SRC_BASE)/lib64/libvixDiskLibVim.so* $(STAGING_DIR_HOST)/lib/
	$(CP) $(HOST_SRC_BASE)/lib64/libgvmomi.so* $(STAGING_DIR_HOST)/lib/
endef

$(eval $(call HostBuild))
