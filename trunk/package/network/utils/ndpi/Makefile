include $(TOPDIR)/rules.mk

PKG_NAME:=ndpi-netfilter
PKG_REV:=5847
PKG_RELEASE:=2
PKG_VERSION:=1.0

PKG_SOURCE:=ndpi-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://svn.ntop.org/svn/ntop/trunk/nDPI
PKG_SOURCE_SUBDIR:=ndpi-netfilter-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=svn

include $(INCLUDE_DIR)/package.mk

#PKG_FIXUP:=autoreconf
PKG_FIXUP:=libtool


define Package/ndpi-netfilter
  SUBMENU:=Firewall
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ndpi successor of OpenDPI
  URL:=http://www.ntop.org/products/ndpi/
  DEPENDS:=+iptables +iptables-mod-conntrack-extra
endef

define Package/ndpi-netfilter/description
	nDPI is a ntop-maintained superset of the popular OpenDPI library
endef

CONFIGURE_VARS:= \
	CC="$(TARGET_CC)" \
	LD="$(TARGET_LD)" \
	LIBTOOL="$(STAGING_DIR)/host/bin/libtool"

CONFIGURE_ARGS := \
	$(filter-out --disable-nls,$(CONFIGURE_ARGS)) \
	--with-pic \

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib/libiconv-full/lib \
		-L$(STAGING_DIR)/usr/lib -lc  -L$(TOOLCHAIN_DIR)

define Build/Prepare
	$(call Build/Prepare/Default)
	rm -rf $(PKG_BUILD_DIR)/ndpi-netfilter
	git clone https://github.com/ewildgoose/ndpi-netfilter $(PKG_BUILD_DIR)/ndpi-netfilter
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(FPIC) -I$(STAGING_DIR)/usr/lib" \
		LD="$(TARGET_LD)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all

	$(MAKE) -C $(PKG_BUILD_DIR)/ndpi-netfilter \
		ARCH="$(LINUX_KARCH)" \
		CC="$(TARGET_CC)" \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib" \
		LD="$(TARGET_LD)" \
		NDPI_PATH=$(PKG_BUILD_DIR) \
		KERNEL_DIR=$(LINUX_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		STAGING_DIR="$(STAGING_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(FPIC) -I$(STAGING_DIR)/usr/include -I$(PKG_BUILD_DIR)/ndpi-netfilter/src -I$(PKG_BUILD_DIR)/src/include" \
		all
endef

define Package/ndpi-netfilter/install
	mkdir -p  $(1)/usr/lib/iptables
	$(CP) $(PKG_BUILD_DIR)/ndpi-netfilter/ipt/libxt_ndpi.so $(1)/usr/lib/iptables
endef


include $(INCLUDE_DIR)/kernel.mk

define KernelPackage/ipt-ndpi
  SUBMENU:=Netfilter Extensions
  TITLE:=nDPI net netfilter module
  DEPENDS:=+kmod-ipt-compat-xtables +libpcap +ndpi-netfilter
  KCONFIG:=CONFIG_NF_CONNTRACK CONFIG_NF_CONNTRACK_EVENTS=y
  FILES:=$(PKG_BUILD_DIR)/ndpi-netfilter/src/xt_ndpi.ko
  AUTOLOAD:=$(call AutoLoad,46,xt_ndpi)
endef

$(eval $(call BuildPackage,ndpi-netfilter))
$(eval $(call KernelPackage,ipt-ndpi))
