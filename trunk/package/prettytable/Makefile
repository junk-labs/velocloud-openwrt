

include $(TOPDIR)/rules.mk

PKG_NAME:=prettytable
PKG_VERSION:=0.7.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/P/PrettyTable
PKG_MD5SUM:=a6b80afeef286ce66733d54a0296b13b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python


include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/prettytable
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=PrettyTable
  URL:=http://pypi.python.org/pypi/prettytable/
  DEPENDS:=+python
endef

define Package/prettytable/description
  Config file reading, writing and validation.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/prettytable/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,prettytable))

