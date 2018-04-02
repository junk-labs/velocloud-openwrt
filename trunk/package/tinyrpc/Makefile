

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyrpc
PKG_VERSION:=0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/source/t/tinyrpc/
PKG_MD5SUM:=4bb9ed559bcccc4bf91a7843da781d28

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/tinyrpc
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python-zmq
  URL:=http://pypi.python.org/pypi/tinyrpc/
  DEPENDS:=+python +requests
endef

define Package/tinyrpc/description
  A small, modular, transport and protocol neutral
  RPC library that, among other things, supports
  JSON-RPC and zmq (zmq is separate in tinyrpc-zmq).
endef

define Package/tinyrpc-zmq
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python-zmq
  URL:=http://pypi.python.org/pypi/tinyrpc/
  DEPENDS:=+python +requests +tinyrpc +zeromq
endef

define Package/tinyrpc-zmq/description
  A small, modular, transport and protocol neutral
  RPC library that, among other things, supports
  JSON-RPC and zmq.
endef

define Build/Configure
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix="$(PKG_INSTALL_DIR)/usr")
endef

define Package/tinyrpc/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)
	rm -f \
		$(1)$(PYTHON_PKG_DIR)/tinyrpc/transports/INTEGRATE_ME.py \
		$(1)$(PYTHON_PKG_DIR)/tinyrpc/transports/zmq.py
endef

define Package/tinyrpc-zmq/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)/tinyrpc/transports
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/tinyrpc/transports/zmq.py \
		$(1)$(PYTHON_PKG_DIR)/tinyrpc/transports
endef

$(eval $(call BuildPackage,tinyrpc))
$(eval $(call BuildPackage,tinyrpc-zmq))

