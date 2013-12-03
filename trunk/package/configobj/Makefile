

include $(TOPDIR)/rules.mk

PKG_NAME:=configobj
PKG_VERSION:=4.7.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/c/configobj
PKG_MD5SUM:=201dbaa732a9049c839f9bb6c27fc7b5
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python


include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/configobj
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=configobj
  URL:=http://pypi.python.org/pypi/configobj/
  DEPENDS:=+python
endef

define Package/configobj/description
  Config file reading, writing and validation.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/configobj/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,configobj))

