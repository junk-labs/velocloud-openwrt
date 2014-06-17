

include $(TOPDIR)/rules.mk

PKG_NAME:=python-zmq
PKG_VERSION:=14.1.1
PKG_RELEASE:=1

PKG_SOURCE:=pyzmq-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://pypi.python.org/packages/source/p/pyzmq/
PKG_MD5SUM:=bea18143c347dcde92cd0409392fbb58

PKG_BUILD_DIR:=$(BUILD_DIR)/pyzmq-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-zmq
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python-zmq
  URL:=http://pypi.python.org/pypi/pyzmq/
  DEPENDS:=+python +zeromq
endef

define Package/python-zmq/description
  Python bindings for 0MQ
endef

define Build/Compile
	$(call Build/Compile/PyMod,.,configure --zmq="bundled")
	$(call Build/Compile/PyMod,.,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/python-zmq/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-zmq))

