# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-oauthlib
PKG_VERSION:=2.0.2
PKG_RELEASE:=1

PKG_SOURCE:=oauthlib-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/fa/2e/25f25e6c69d97cf921f0a8f7d520e0ef336dd3deca0142c0b634b0236a90/
PKG_MD5SUM=f59b448a30684347e00f369bc3542abc
PKG_BUILD_DEPENDS:=python
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Ib Lundgren

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-oauthlib
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:= https://github.com/stefankoegl/python-json-patch
  TITLE:=python-oauthlib
  DEPENDS:=+python
endef

define Package/python-oauthlib/description
OAuthLib is a generic utility which implements the logic of OAuth without assuming a specific HTTP request object or web framework.
endef

define Build/Compile
        $(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-oauthlib/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
            $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
            $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-oauthlib))
