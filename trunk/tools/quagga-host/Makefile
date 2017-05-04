# Copyright (C) 2016 velocloud.net
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quagga-host
PKG_REV:=0.99.24.1
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=15

PKG_SOURCE_PROTO:=none
PKG_SOURCE_URL:=none
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)/build

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

ORIG_SOURCE_DIR=../../package/quagga
HOST_UNPACK:=( rsync -a $(ORIG_SOURCE_DIR)/src/ $(HOST_BUILD_DIR) ; \
               rsync -a $(ORIG_SOURCE_DIR)/debian $(HOST_BUILD_DIR) ; \
               sh ./files/fix-changelog.sh "$(PKG_VERSION)-$(PKG_RELEASE)" $(HOST_BUILD_DIR)/debian/changelog )

define Host/Configure
	echo No Configure
endef

define Host/Compile
	(cd $(HOST_BUILD_DIR) && \
	 debuild -i -us -uc -b -d \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/hostpkg
	$(CP) $(HOST_BUILD_DIR)/../*.deb $(STAGING_DIR_HOST)/hostpkg/
endef

$(eval $(call HostBuild))
