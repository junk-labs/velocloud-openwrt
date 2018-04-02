

include $(TOPDIR)/rules.mk

PKG_NAME:=cloud-init
PKG_VERSION:=0.7.2
PKG_RELEASE:=1


PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://launchpad.net/$(PKG_NAME)/trunk/$(PKG_VERSION)/+download/
PKG_MD5SUM:=53ee21e7b7b3f42ce50d1303a484fe28
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python



include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/cloud-init
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=cloud-init
  URL:=http://pypi.python.org/pypi/cloud-init/
  DEPENDS:=+python +boto +Cheetah +configobj +pyyaml +prettytable
endef

define Package/cloud-init/description
  Amazon Web Services Library
endef

define Build/Compile
	$(call Build/Compile/PyMod,, install --prefix="$(PKG_INSTALL_DIR)/usr" \
					     --install-data="$(PKG_INSTALL_DIR)")
endef


define Package/cloud-init/conffiles
/etc/cloud/cloud.cfg
/etc/cloud/cloud.cfg.d/05_logging.cfg
endef

define Package/cloud-init/postinst
#! /bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Enabling rc.d symlink for cloud-init"
	/etc/init.d/cloud-init enable
	/etc/init.d/cloud-init-local enable
	/etc/init.d/cloud-config enable
	/etc/init.d/cloud-final enable
fi
exit 0
endef

define Package/cloud-init/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)/etc \
		$(1)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr \
		$(1)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/init.d/cloud-init $(1)/etc/init.d/cloud-init
	$(INSTALL_DATA) ./files/cloud.cfg $(1)/etc/cloud/cloud.cfg
endef

$(eval $(call BuildPackage,cloud-init))

