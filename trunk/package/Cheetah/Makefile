

include $(TOPDIR)/rules.mk

PKG_NAME:=Cheetah
PKG_VERSION:=2.4.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/C/Cheetah
PKG_MD5SUM:=853917116e731afbc8c8a43c37e6ddba
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python


include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/Cheetah
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Cheetah
  URL:=http://pypi.python.org/pypi/Cheetah/
  DEPENDS:=+python
endef

define Package/Cheetah/description
  Cheetah is a template engine and code generation tool.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/Cheetah/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,Cheetah))

