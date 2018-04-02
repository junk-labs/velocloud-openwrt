#
# Copyright (C) 2012 Michael Crosson <mcrosson_openwrt@nusku.net>
# Copyright (C) 2011-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ca-certificates
# NOTE: This version keeps changing. If the build fails, check the contents
# of http://ftp.debian.org/debian/pool/main/c/ca-certificates/ and update it.
PKG_VERSION:=20141019
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION)_all.deb
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/c/ca-certificates
PKG_MD5SUM:=cbac2b1e7259c11571ad4393ee84a190

UNPACK_CMD=ar -p "$(DL_DIR)/$(PKG_SOURCE)" data.tar.xz | ( cd $(1) && tar -xJf - )

include $(INCLUDE_DIR)/package.mk

define Package/ca-certificates
  SECTION:=admin
  CATEGORY:=SSL
  TITLE:=Root CA Certificates
  URL:=http://packages.debian.org/sid/ca-certificates
  MAINTAINER:=Michael Crosson <mcrosson_openwrt@nusku.net>
endef

define Package/ca-certificates/description
 the ca-certificates published by the Debian project
endef

define Build/Compile
	# Do nothing
endef

define Package/ca-certificates/install
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usr/share/ca-certificates/*/*.crt $(1)/etc/ssl/certs/
endef

define Package/ca-certificates/postinst
#!/bin/sh
echo $${IPKG_INSTROOT}
for FILE in `find $${IPKG_INSTROOT}/etc/ssl/certs -iname *.crt`; do
	HASH=`openssl x509 -hash -noout -in $$FILE`;
	BASENAME=`basename $$FILE`;
	ln -sf ./$$BASENAME $${IPKG_INSTROOT}/etc/ssl/certs/"$$HASH.0";
done
endef

$(eval $(call BuildPackage,ca-certificates))
