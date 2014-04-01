#
# OpenWrt build Makefile for Mocana NanoSec stack.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mocana-nanosec
PKG_REV:=5.8.0-33895
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=1

# Base name of the Mocana-distributed Source Zip.
# Within it, it contains a top-level dir with the same name.
# When picking up a new source distro, update this and the
# PKG_REV above.
MOCANA_PKG_BASE=mss_2013_10_31_33895

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_REV)
PKG_SOURCE_URL:=none
PKG_SOURCE:=$(MOCANA_PKG_BASE).zip
PKG_SOURCE_DIR:=$(PKG_BUILD_DIR)/$(MOCANA_PKG_BASE)
PKG_UNPACK:=unzip -q -P mocana -d $(PKG_BUILD_DIR) $(PKG_SOURCE) && \
	unzip -q -P mocana -d $(PKG_BUILD_DIR) missiu_33895.zip && \
	cp Makefile.mocana $(PKG_SOURCE_DIR)/Makefile
PKG_MAINTAINER:=Shankar Unni <shankar@velocloud.net>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mocana-ipsec
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Mocana IPSec driver
  FILES:=$(PKG_SOURCE_DIR)/bin/moc_ipsec.ko $(PKG_SOURCE_DIR)/bin/moc_memdrv.ko
  AUTOLOAD:=$(call AutoLoad,75,moc_ipsec moc_memdrv)
  URL:=http://velocloud.net
endef

define KernelPackage/mocana-ipsec/description
 Driver for IPsec from Mocana
endef

define Package/mocana-nanosec
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Mocana NanoSec IPSec stack
  URL:=http://velocloud.net
endef

define Package/mocana-nanosec/description
 Mocana NanoSec IPSec stack
endef

define Package/mocana-nanosec/install
	$(INSTALL_DIR) $(1)/opt/vc/sbin
	$(CP) $(PKG_SOURCE_DIR)/bin/{ike,loadConfig} $(1)/opt/vc/sbin/
	#$(INSTALL_DATA) $(PKG_SOURCE_DIR)/bin/{sample_config} $(1)/etc/config/ipsec
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib 	\
		$(1)/usr/include \
		$(1)/usr/include/mocana \
		$(1)/usr/include/mocana/common \
		$(1)/usr/include/mocana/crypto \
		$(1)/usr/include/mocana/ike \
		$(1)/usr/include/mocana/ike2 \
		$(1)/usr/include/mocana/ipsec \
		$(1)/usr/include/mocana/platform
	$(CP) $(PKG_SOURCE_DIR)/bin/libikeipsec.a $(1)/usr/lib
	$(CP) $(PKG_SOURCE_DIR)/src/common/*.h $(1)/usr/include/mocana/common
	$(CP) $(PKG_SOURCE_DIR)/src/crypto/*.h $(1)/usr/include/mocana/crypto
	$(CP) $(PKG_SOURCE_DIR)/src/ike/*.h $(1)/usr/include/mocana/ike
	$(CP) $(PKG_SOURCE_DIR)/src/ike2/*.h $(1)/usr/include/mocana/ike2
	$(CP) $(PKG_SOURCE_DIR)/src/ipsec/*.h $(1)/usr/include/mocana/ipsec
	$(CP) $(PKG_SOURCE_DIR)/src/platform/*.h $(1)/usr/include/mocana/platform
endef

define Build/Configure
endef

ifeq ($(CONFIG_ARCH_64BIT),y)
MOCANA_64_BIT="-D__ENABLE_MOCANA_64_BIT__="
endif

define Build/Compile
	$(MAKE) -C $(PKG_SOURCE_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		CXX="$(TARGET_CROSS)gcc" \
		KLD="$(TARGET_CROSS)ld" \
		LD="$(TARGET_CROSS)gcc" \
		KDIR="$(LINUX_DIR)" \
		MOCANA_64_BIT=$(MOCANA_64_BIT) \
		all
endef

$(eval $(call BuildPackage,mocana-nanosec))
$(eval $(call KernelPackage,mocana-ipsec))
