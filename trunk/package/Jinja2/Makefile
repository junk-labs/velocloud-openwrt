# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=Jinja2
PKG_VERSION:=2.9.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/90/61/f820ff0076a2599dd39406dcb858ecb239438c02ce706c8e91131ab9c7f1/
PKG_HASH:=ddaa01a212cd6d641401cb01b605f4a4d9f37bfc93043d7f760ec70fb99ff9ff
PKG_BUILD_DEPENDS:=python
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-jinja2
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=http://jinja.pocoo.org/
  TITLE:=python-jinja2
  DEPENDS:=+python +python-markupsafe
endef

define Package/python-jinja2/description
Jinja2 is a full featured template engine for Python. It has full
unicode support, an optional integrated sandboxed execution
environment, widely used and BSD licensed.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-jinja2/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
            $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
            $(1)$(PYTHON_PKG_DIR)
endef


$(eval $(call BuildPackage,python-jinja2))
