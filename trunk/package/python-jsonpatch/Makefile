# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-jsonpatch
PKG_VERSION:=1.16
PKG_RELEASE:=1

PKG_SOURCE:=jsonpatch-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/36/de/499bea7aac917f86eb5be148f631c3ddced4e60c8d119d63939c53a5ab5b/
PKG_MD5SUM=8ef1ceb00dcf992c9e43611f698f9279
PKG_BUILD_DEPENDS:=python
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Stefan Ko¶gl

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-jsonpatch
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:= https://github.com/stefankoegl/python-json-patch
  TITLE:=python-jsonpatch
  DEPENDS:=+python +python-jsonpointer
endef

define Package/python-jsonpatch/description
Python library to apply JSON Patches according to RFC 6902
endef

define Build/Compile
        $(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-jsonpatch/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
            $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
            $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-jsonpatch))
