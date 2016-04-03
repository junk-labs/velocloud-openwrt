include $(TOPDIR)/rules.mk

PKG_NAME:=libqmi
PKG_VERSION:=1.16.2
PKG_RELEASE:=1

PKG_SOURCE:=libqmi-$(PKG_VERSION).tar.xz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=f114f4ebe66c538cbb28b05eb0b065fe08c6f9fa
PKG_SOURCE_URL:=git://anongit.freedesktop.org/libqmi
PKG_SOURCE_SUBDIR:=$(PKG_NAME)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libqmi
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +zlib +kmod-usb-net-qmi-wwan +libmbim
  TITLE:=Helper library and utils to talk to QMI enabled modems
  URL:=http://www.freedesktop.org/wiki/Software/libqmi
endef

define Package/libqmi/description
  Helper library and utils to talk to QMI enabled modems
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libqmi-glib \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/qmi-glib.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libqmi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/qmi-proxy $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/qmicli $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/qmi-network $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libqmi))
