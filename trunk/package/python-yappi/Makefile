#
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-yappi
PKG_VERSION:=0.62
PKG_RELEASE:=1

PKG_SOURCE:=yappi-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://yappi.googlecode.com/files/
PKG_MD5SUM:=a1b3d3e01053d0b8ebb8debef62d8960

PKG_BUILD_DIR:=$(BUILD_DIR)/yappi-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-yappi
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python-yappi
  URL:=https://yappi.googlecode.com/files/
  DEPENDS:=+python
endef

define Package/python-cjson/description
  Multi-thread-aware profiler for Python
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/python-yappi/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-yappi))
