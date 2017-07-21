# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-jsonpointer
PKG_VERSION:=1.10
PKG_RELEASE:=1

PKG_SOURCE:=jsonpointer-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/f6/36/6bdd302303e8bc7c25102dbc1eabb3e3d97f57b0f8f414f4da7ea7ab9dd8/
PKG_MD5SUM=d68c0c6ad6889e9c94ec0feba719e45e
PKG_BUILD_DEPENDS:=python
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Stefan Kög

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-jsonpointer
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:= https://github.com/stefankoegl/python-json-pointer
  TITLE:=python-jsonpointer
  DEPENDS:=+python
endef

define Package/python-jsonpointer/description
Library to resolve JSON Pointers according to
[RFC 6901](http://tools.ietf.org/html/rfc6901)
endef

define Build/Compile
        $(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-jsonpointer/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
            $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
            $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-jsonpointer))
