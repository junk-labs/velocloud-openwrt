

include $(TOPDIR)/rules.mk

PKG_NAME:=requests
PKG_VERSION:=1.2.3
PKG_RELEASE:=1


PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/r/requests/
PKG_MD5SUM:=adbd3f18445f7fe5e77f65c502e264fb

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/requests
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=requests
  URL:=http://pypi.python.org/pypi/requests/
  DEPENDS:=+python
endef

define Package/requests/description
  Python HTTP for Humans.
endef

define Build/Configure
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr") 
endef

define Package/requests/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,requests))

