#
# Copyright (C) 2015 VeloCloud Networks, Inc.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dpdk
PKG_VERSION:=17.02
PKG_RELEASE:=1
PKG_FULL_VERSION=$(PKG_VERSION)-$(PKG_RELEASE)

PKG_GIT_URL=git@git.assembla.com:velocloud.dpdk.git
PKG_GIT_COMMIT=62635ad939a52ae999775261b09705abd0b98e28

PKG_GIT_DIR=$(TOPDIR)/git-src/dpdk
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

# do the host build as well if this package is selected
PKG_BUILD_DEPENDS:=dpdk/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

# exclude git files from dependency timestamp
DEP_FINDPARAMS := $(DEP_FINDPARAMS) -x "*/.git/*"

#
# host build
#
HOST_PKGSRC_DIR:=$(HOST_BUILD_DIR)/package/usr/src/dpdk-$(PKG_VERSION)

define LinkDir
	rm -fr $1/*
	mkdir -p $1
	cp -al $(PKG_GIT_DIR)/* $1/
endef

define Host/Prepare/Default
	mkdir -p $(dir $(PKG_GIT_DIR))
	[ -d $(PKG_GIT_DIR)/.git ] || git clone $(PKG_GIT_URL) $(PKG_GIT_DIR)
	(cd $(PKG_GIT_DIR) && (git rev-parse --quiet --verify $(PKG_GIT_COMMIT)^{commit} || git fetch --all))
	(cd $(PKG_GIT_DIR) && git checkout $(PKG_GIT_COMMIT))
endef

define Host/Configure
	$(call LinkDir, $(HOST_PKGSRC_DIR))
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/dkms.conf > $(HOST_PKGSRC_DIR)/dkms.conf
	mkdir -p $(HOST_BUILD_DIR)/package/DEBIAN
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/DEBIAN/postinst > $(HOST_BUILD_DIR)/package/DEBIAN/postinst
	chmod 0755 $(HOST_BUILD_DIR)/package/DEBIAN/postinst
	sed "s/__BASE_VERSION__/$(PKG_VERSION)/g" ./files/DEBIAN/prerm > $(HOST_BUILD_DIR)/package/DEBIAN/prerm
	chmod 0755 $(HOST_BUILD_DIR)/package/DEBIAN/prerm
	sed "s/__FULL_VERSION__/$(PKG_FULL_VERSION)/g" ./files/DEBIAN/control > $(HOST_BUILD_DIR)/package/DEBIAN/control
endef

define Host/Compile
	(cd $(HOST_BUILD_DIR)/package && dpkg-deb -Zxz -b . ../gatewayd-dpdk_$(PKG_FULL_VERSION)_all.deb)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/hostpkg
	$(CP) $(HOST_BUILD_DIR)/*.deb $(STAGING_DIR_HOST)/hostpkg
endef

#
# target package build
#
define Package/dpdk
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Intel Data Plane Development Kit
  URL:=http://dpdk.org/
  DEPENDS:=@i386||x86_64 +libpcap +libpthread +librt +kmod-hwmon-core
endef

define Package/dpdk/description
  Intel Data Plane Development Kit.
endef

define Package/dpdk-util
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Intel Data Plane Development Kit utilities
  URL:=http://dpdk.org/
  DEPENDS:=@i386||x86_64 +dpdk
endef

define Package/dpdk-util/description
  Intel Data Plane Development Kit utilities.
endef

define Download/default
endef

define Build/Prepare/Default
	mkdir -p $(PKG_BUILD_DIR)
endef

TARCH=$(patsubst i386,i686,$(ARCH))
T=$(TARCH)-openwrt-linuxapp-gcc
DPDK_DEBUG=-g

MAKE_FLAGS = -C $(PKG_GIT_DIR) \
		RTE_OUTPUT=$(PKG_BUILD_DIR) \
		T=$(T) V=$(V) \
		CONFIG_RTE_BUILD_COMBINE_LIBS=y \
		CROSS="$(TARGET_CROSS)" \
		OPENWRT_CFLAGS="$(TARGET_CFLAGS) $(DPDK_DEBUG) -msse3 -msse4" \
		OPENWRT_LDFLAGS="$(TARGET_LDFLAGS) $(DPDK_DEBUG) " \
		OPENWRT_LDLIBS="$(TARGET_LDLIBS)" \
		RTE_KERNELDIR="$(LINUX_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)"

define Build/Compile
	$(MAKE) $(PKG_JOBS) $(MAKE_FLAGS) config
	$(MAKE) $(PKG_JOBS) $(MAKE_FLAGS) depdirs
	$(MAKE) $(PKG_JOBS) $(MAKE_FLAGS) install
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/dpdk/include \
		$(1)/usr/dpdk/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/include/dpdk/* \
		$(1)/usr/dpdk/include
	$(CP) \
		$(PKG_INSTALL_DIR)/lib/*.a \
		$(1)/usr/dpdk/lib
	$(LN) \
		libdpdk.a \
		$(1)/usr/dpdk/lib/libintel_dpdk.a
endef

define Package/dpdk/install
	$(INSTALL_DIR) \
		$(1)/usr/share/dpdk/kmod
	$(CP) \
		$(PKG_INSTALL_DIR)/lib/modules/*/extra/dpdk/*.ko \
		$(1)/usr/share/dpdk/kmod
endef

define Package/dpdk-util/install
	$(INSTALL_DIR) \
		$(1)/usr/share/dpdk/bin
	$(INSTALL_BIN) -s \
		$(PKG_INSTALL_DIR)/bin/dpdk-pdump \
		$(1)/usr/share/dpdk/bin
	$(INSTALL_BIN) -s \
		$(PKG_INSTALL_DIR)/bin/dpdk-procinfo \
		$(1)/usr/share/dpdk/bin
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,dpdk))
$(eval $(call BuildPackage,dpdk-util))
