
include $(TOPDIR)/rules.mk

PKG_NAME:=cprops
PKG_VERSION:=0.1.12
PKG_RELEASE:=1



PKG_BUILD_DIR=$(BUILD_DIR)/libcprops-$(PKG_VERSION)
PKG_SOURCE:=libcprops-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_MD5SUM:=0c209fb65198b41f89e902001d1cff8a
PKG_CAT:=zcat

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=autogen.sh aclocal.m4

include $(INCLUDE_DIR)/package.mk


define Package/cprops
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=c prototyping tools
  DEPENDS:=+libpthread
  URL:=http://cprops.sourceforge.net
endef

define Package/cprops/description
	The c prototyping tools library provides generic tools for
	application development in plain c covering basic data
	structure implementations, persistence, threading and tcp
	and http communication.
	  o data structures
		- bit string
		- linked list
		- heap
		- priority list
		- hashtable
		- hashlist
		- ordered hash
		- avltree
		- red-black tree
		- splay tree
		- N-ary tree
		- character trie, wide character trie, bit trie
		- multimap - multiple index map

	   o application level components
		- cp_mempool, cp_shared_mempool - memory pool
		- cp_thread - thread pool
		- cp_client - tcp client socket
		- cp_socket - tcp server socket
		- cp_httpclient - http client socket
		- cp_httpsocket - http server socket
		- cp_dbms - a database abstraction layer, currently
		            supporting postgres and mysql

	libcprops provides a simple and concise api designed for use
	in multi-threaded applications. libcprops components exhibit
	good performance characteristics and a small memory footprint.
	Data structure behavior may be controlled at run time through
	a mode parameter to enable or disable duplicate entries, memory
	management and synchronization. Collection operations can be
	unsynchronized, individually synchronized or externally
	synchronized (transaction-like).
endef


TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	  --disable-cpsp \
	  --disable-cpsvc

define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cprops $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcprops.{a,so*,la} $(1)/usr/lib
endef

define Package/cprops/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcprops.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,cprops))
