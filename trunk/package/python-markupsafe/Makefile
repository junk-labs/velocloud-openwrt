# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-markupsafe
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=MarkupSafe-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/4d/de/32d741db316d8fdb7680822dd37001ef7a448255de9699ab4bfcbdf4172b
PKG_MD5SUM=2fcedc9284d50e577b5192e8e3578355
PKG_BUILD_DEPENDS:=python
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Armin Ronacher

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-markupsafe
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:= http://github.com/pallets/markupsafe
  TITLE:=python-markupsafe
  DEPENDS:=+python
endef

define Package/python-markupsafe/description
Implements a XML/HTML/XHTML Markup safe string for Python
endef

define Build/Compile
        $(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-markupsafe/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
            $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
            $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-markupsafe))
