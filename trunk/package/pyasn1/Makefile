

include $(TOPDIR)/rules.mk

PKG_NAME:=pyasn1
PKG_VERSION:=0.1.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/p/pyasn1/
PKG_MD5SUM:=e1851ac1123c50f134cc29420fd5e0bd
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python


include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/pyasn1
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=pyasn1
  URL:=http://pypi.python.org/pypi/pyasn1/
  DEPENDS:=+python
endef

define Package/pyasn1/description
  ASN.1 types and codecs
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/pyasn1/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,pyasn1))

