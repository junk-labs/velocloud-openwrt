#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-six
PKG_VERSION:=1.10.0
PKG_RELEASE:=1

PKG_SOURCE:=six-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/s/six
PKG_MD5SUM:=34eed507548117b2ab523ab14b2f8b55

PKG_BUILD_DEPENDS:=python
PKG_BUILD_DIR:=$(BUILD_DIR)/six-$(PKG_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-six
	SECTION:=lang
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=python-six
	URL:=https://pypi.python.org/pypi/six
	DEPENDS:=+python-light
endef

define Package/python-six/description
Six is a Python 2 and 3 compatibility library.  It provides utility functions
for smoothing over the differences between the Python versions with the goal of
writing Python code that is compatible on both Python versions.  See the
documentation for more information on what is provided.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/python-six/install
	echo $(1)$(PYTHON_PKG_DIR) $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-six))
