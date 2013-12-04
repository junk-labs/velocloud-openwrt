

include $(TOPDIR)/rules.mk

PKG_NAME:=ndg-httpsclient
PKG_VERSION:=0.3.2
PKG_RELEASE:=1

PKG_SOURCE:=ndg_httpsclient-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/n/ndg-httpsclient/
PKG_MD5SUM:=076303c7aa0e41f3b45a7cb43dbb0743
PKG_BUILD_DIR:=$(BUILD_DIR)/ndg_httpsclient-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/ndg-httpsclient
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=ndg-httpsclient
  URL:=http://pypi.python.org/pypi/pyzmq/
  DEPENDS:=+python
endef

define Package/ndg-httpsclient/description
  Provides enhanced HTTPS support for httplib
  and urllib2 using PyOpenSSL
endef

define Build/Compile
	$(call Build/Compile/PyMod,.,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/ndg-httpsclient/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,ndg-httpsclient))

