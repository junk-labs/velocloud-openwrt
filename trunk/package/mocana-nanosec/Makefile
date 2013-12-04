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


define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_SOURCE_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		CXX="$(TARGET_CROSS)gcc" \
		KLD="$(TARGET_CROSS)ld" \
		LD="$(TARGET_CROSS)gcc" \
		KDIR="$(LINUX_DIR)" \
		all
endef

$(eval $(call BuildPackage,mocana-nanosec))
$(eval $(call KernelPackage,mocana-ipsec))
