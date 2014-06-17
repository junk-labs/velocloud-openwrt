
include $(TOPDIR)/rules.mk

PKG_NAME:=czmq
PKG_VERSION:=2.1.0
PKG_RELEASE:=1



PKG_BUILD_DIR=$(BUILD_DIR)/czmq-$(PKG_VERSION)
PKG_SOURCE:=czmq-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.zeromq.org/
PKG_MD5SUM:=068598475a5c47e399f3fa298a770000
PKG_CAT:=zcat

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk


define Package/czmq
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=c prototyping tools
  DEPENDS:=+zeromq
  URL:=http://czmq.sourceforge.net
endef

define Package/czmq/description
	CZMQ has these goals:
	  - To wrap the ØMQ core API in semantics that are natural
	    and lead to shorter, more readable applications.
	  - To hide the differences between versions of ØMQ.
	  - To provide a space for development of more sophisticated
	    API semantics.
endef


TARGET_CFLAGS = -std=c99 -Wno-error -pedantic

define Build/Configure
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib 	\
		$(1)/usr/include \
		$(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig \
		$(1)/usr/lib/
endef

define Package/czmq/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,czmq))
