# 
# Copyright (C) 2013 Julius Schulz-Zander
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile $

include $(TOPDIR)/rules.mk

PKG_NAME:=open-vm-tools

PKG_RELEASE:=1
PKG_VERSION:=10.0.5-3227872

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/open-vm-tools/open-vm-tools/stable-10.0.x/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_BUILD_DEPENDS:=libdnet procps

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/open-vm-tools
  SECTION:=utils
  CATEGORY:=VMware
  TITLE:=VMware Tools for OpenWRT
  URL:=@SF/open-vm-tools/
  DEPENDS:=@TARGET_x64_vc_vmdk +libdnet +procps +glib2
  DEFAULT:=y if (TARGET_x64_vc_vmdk)
endef

define Package/open-vm-tools/description
  Package for open-vm-tools.
endef

CONFIGURE_ARGS = \
	RPCGEN=/usr/bin/rpcgen \
	--prefix=/usr \
	--disable-deploypkg \
	--disable-docs \
	--disable-tests \
	--disable-grabbitmqproxy \
	--disable-vgauth \
	--without-kernel-modules \
	--without-pam \
	--without-x \
	--without-icu \
	--without-gtk2 \
	--without-gtkmm \
	--without-xmlsecurity \


define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/InstallDev
	echo FOO
endef

define Package/open-vm-tools/install
	$(INSTALL_DIR) $(1)/etc/vmware-tools
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/resume-vm-default \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/poweron-vm-default \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/suspend-vm-default \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/vm-support \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/statechange.subr \
		$(PKG_INSTALL_DIR)/etc/vmware-tools/poweroff-vm-default \
		$(1)/etc/vmware-tools
	$(INSTALL_DIR) $(1)/etc/vmware-tools/scripts/vmware
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/vmware-toolbox-cmd \
		$(PKG_INSTALL_DIR)/usr/bin/vmware-xferlogs \
		$(PKG_INSTALL_DIR)/usr/bin/vmware-checkvm \
		$(PKG_INSTALL_DIR)/usr/bin/vmtoolsd \
		$(PKG_INSTALL_DIR)/usr/bin/vmware-rpctool \
		$(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libguestlib.so.0.0.0 \
		$(PKG_INSTALL_DIR)/usr/lib/libhgfs.so.0.0.0 \
		$(PKG_INSTALL_DIR)/usr/lib/libvmtools.so.0.0.0 \
		$(1)/usr/lib
	ln -sf libguestlib.so.0.0.0 $(1)/usr/lib/libguestlib.so.0
	ln -sf libguestlib.so.0.0.0 $(1)/usr/lib/libguestlib.so
	ln -sf libhgfs.so.0.0.0 $(1)/usr/lib/libhgfs.so.0
	ln -sf libhgfs.so.0.0.0 $(1)/usr/lib/libhgfs.so
	ln -sf libvmtools.so.0.0.0 $(1)/usr/lib/libvmtools.so.0
	ln -sf libvmtools.so.0.0.0 $(1)/usr/lib/libvmtools.so
	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/common
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/common/libhgfsServer.so \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/common/libvix.so \
		$(1)/usr/lib/open-vm-tools/plugins/common
	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/vmsvc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libtimeSync.so \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libvmbackup.so \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libguestInfo.so \
		$(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libpowerOps.so \
		$(1)/usr/lib/open-vm-tools/plugins/vmsvc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/openvmtools.init $(1)/etc/init.d/openvmtools
endef

$(eval $(call BuildPackage,open-vm-tools))
