

include $(TOPDIR)/rules.mk

PKG_NAME:=boto
PKG_VERSION:=2.9.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/b/boto
PKG_MD5SUM:=cc79cc87edb4cce00ca275c8ea7b75ed
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python


include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/boto
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=boto
  URL:=http://pypi.python.org/pypi/boto/
  DEPENDS:=+python
endef

define Package/boto/description
  Amazon Web Services Library
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/boto/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,boto))

