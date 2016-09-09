# Copyright (C) 2016 velocloud.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dpdk-host
PKG_VERSION:=16.07
PKG_RELEASE:=2

PKG_SOURCE:=dpdk-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dpdk.org/browse/dpdk/snapshot/
PKG_MD5SUM:=4afdc7951e21ff878a85ecade7f6f488

include $(INCLUDE_DIR)/host-build.mk

HOST_PKGSRC_DIR:=$(HOST_BUILD_DIR)/package/usr/src/dpdk-$(PKG_VERSION)

HOST_UNPACK:=(mkdir -p $(HOST_PKGSRC_DIR) && tar -C $(HOST_PKGSRC_DIR)/.. -xf $(DL_DIR)/$(PKG_SOURCE))

define Host/Patch
	$(call HostPatchDir,$(HOST_PKGSRC_DIR),$(HOST_PATCH_DIR),)
endef

VERSTR=$(PKG_VERSION)-$(PKG_RELEASE)

define Host/Configure
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/dkms.conf > $(HOST_PKGSRC_DIR)/dkms.conf
	mkdir -p $(HOST_BUILD_DIR)/package/DEBIAN
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/DEBIAN/postinst > $(HOST_BUILD_DIR)/package/DEBIAN/postinst
	chmod 0755 $(HOST_BUILD_DIR)/package/DEBIAN/postinst
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/DEBIAN/prerm > $(HOST_BUILD_DIR)/package/DEBIAN/prerm
	chmod 0755 $(HOST_BUILD_DIR)/package/DEBIAN/prerm
	sed "s/__FULL_VERSION__/$(VERSTR)/g" ./files/DEBIAN/control > $(HOST_BUILD_DIR)/package/DEBIAN/control
endef

define Host/Compile
	(cd $(HOST_BUILD_DIR)/package && dpkg-deb -Zxz -b . ../gatewayd-dpdk_$(VERSTR)_all.deb)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/hostpkg
	$(CP) $(HOST_BUILD_DIR)/*.deb $(STAGING_DIR_HOST)/hostpkg
endef

$(eval $(call HostBuild))
