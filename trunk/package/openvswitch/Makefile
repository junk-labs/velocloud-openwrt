# 
# Copyright (C) 2013 Julius Schulz-Zander
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile $

include $(TOPDIR)/rules.mk

PKG_NAME:=openvswitch

PKG_RELEASE:=1
PKG_VERSION:=2.6.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://openvswitch.org/releases/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=python/host python-six/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_FIXUP=libtool

define Package/openvswitch/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.openvswitchswitch.org/
  DEPENDS:=+libopenssl +librt +libpcap
  MAINTAINER:=Julius Schulz-Zander <julius@net.t-labs.tu-berlin.de>
endef

define Package/openvswitch/Default/description
  Open vSwitch is a production quality, multilayer, software-based, Ethernet
  virtual switch. It is designed to enable massive network automation through
  programmatic extension, while still supporting standard management interfaces
  and protocols (e.g. NetFlow, sFlow, SPAN, RSPAN, CLI, LACP, 802.1ag). In
  addition, it is designed to support distribution across multiple physical
  servers similar to VMware's vNetwork distributed vswitch or Cisco's Nexus
  1000V.
endef

define Package/openvswitch-common
  $(call Package/openvswitch/Default)
  TITLE:=Open Flow Switch Userspace Package
  DEPENDS:=+libpcap +libopenssl +librt +kmod-openvswitch +dpdk
endef

define Package/openvswitch-common/description
  openvswitch-common provides components required by both openvswitch-switch and
endef

define Package/openvswitch-ipsec
  $(call Package/openvswitch/Default)
  TITLE:=Open Flow Switch Userspace Package
  DEPENDS:=+openvswitch-common
endef

define Package/openvswitch-ipsec/description
  The ovs-monitor-ipsec script provides support for encrypting GRE tunnels with 
  IPsec.
endef

define Package/openvswitch-switch
  $(call Package/openvswitch/Default)
  TITLE:=Open Flow Switch Userspace Package
  DEPENDS:=+openvswitch-common
endef

define Package/openvswitch-switch/description
  openvswitch-switch provides the userspace components and utilities for the
  Open vSwitch kernel-based switch.
endef

define KernelPackage/openvswitch
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  TITLE:=Open Flow Data Path Drivers
  KCONFIG:=CONFIG_BRIDGE
  DEPENDS:=+kmod-stp @IPV6 +kmod-gre +kmod-lib-crc32c \
           +kmod-ipt-conntrack +kmod-ipt-nat6 +kmod-ipt-conntrack-extra +kmod-nf-conntrack-netlink
  FILES:= \
	$(PKG_BUILD_DIR)/datapath/linux/openvswitch.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/datapath/linux/vport-stt.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/datapath/linux/vport-gre.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/datapath/linux/vport-vxlan.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/datapath/linux/vport-lisp.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/datapath/linux/vport-geneve.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,21,openvswitch)
endef

define KernelPackage/openvswitch/description
  This package contains the Open vSwitch kernel moodule and bridge compat
  module. Furthermore, it supports OpenFlow.
endef

CONFIGURE_ARGS += --with-linux=$(LINUX_DIR)
CONFIGURE_ARGS += --with-rundir=/var/run
CONFIGURE_ARGS += --enable-ndebug
CONFIGURE_ARGS += --with-dpdk=$(STAGING_DIR)/usr/dpdk
CONFIGURE_ARGS += PATH="$(TARGET_PATH)"

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		autoreconf -v --install --force || exit 1 \
	);
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="-I$(PKG_BUILD_DIR)/lib $(TARGET_CFLAGS) -std=gnu99" \
		LDFLAGS="-L$(PKG_BUILD_DIR)/lib $(TARGET_LDFLAGS)" \
		LDFLAGS_MODULES="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/lib" \
		STAGING_DIR="$(STAGING_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/datapath/linux" \
		PATH="$(TARGET_PATH)" \
		KCC="$(KERNEL_CC)"
endef

define Build/Install
	$(MAKE) install -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="-I$(PKG_BUILD_DIR)/lib $(TARGET_CFLAGS) -std=gnu99" \
		LDFLAGS="-L$(PKG_BUILD_DIR)/lib $(TARGET_LDFLAGS)" \
		LDFLAGS_MODULES="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/lib" \
		STAGING_DIR="$(STAGING_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/datapath/linux" \
		PATH="$(TARGET_PATH)" \
		KCC="$(KERNEL_CC)"
endef

define Package/openvswitch-ipsec/install
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/scripts
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/debian/ovs-monitor-ipsec $(1)/usr/share/openvswitch/scripts
endef

define Package/openvswitch-common/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/openvswitch.init $(1)/etc/init.d/openvswitch

	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-appctl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-docker $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-ofctl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-parse-backtrace $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-pki $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovsdb-client $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ovs-bugtool $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/share/openvswitch/scripts
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/scripts/ovs-lib $(1)/usr/share/openvswitch/scripts/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/scripts/ovs-bugtool-* $(1)/usr/share/openvswitch/scripts/
	
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/bugtool-plugins
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/openvswitch/bugtool-plugins/system-configuration.xml $(1)/usr/share/openvswitch/bugtool-plugins/
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/bugtool-plugins/network-status
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/openvswitch/bugtool-plugins/network-status/openvswitch.xml $(1)/usr/share/openvswitch/bugtool-plugins/network-status/
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/bugtool-plugins/system-configuration
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/openvswitch/bugtool-plugins/system-configuration/openvswitch.xml $(1)/usr/share/openvswitch/bugtool-plugins/system-configuration/
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/bugtool-plugins/kernel-info
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/openvswitch/bugtool-plugins/kernel-info/openvswitch.xml $(1)/usr/share/openvswitch/bugtool-plugins/kernel-info/
	$(INSTALL_DIR) $(1)/usr/share/openvswitch/bugtool-plugins/system-logs
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/openvswitch/bugtool-plugins/kernel-info/openvswitch.xml $(1)/usr/share/openvswitch/bugtool-plugins/kernel-info/

endef

define Package/openvswitch-common/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || /etc/init.d/openvswitch enable || true
endef

define Package/openvswitch-switch/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-dpctl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-dpctl-top $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-pcap $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-tcpdump $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-tcpundump $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-vlan-test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovs-vsctl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovsdb-tool $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ovs-vswitchd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ovsdb-server $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/share/openvswitch/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/vswitch.ovsschema $(1)/usr/share/openvswitch/

	$(INSTALL_DIR) $(1)/usr/share/openvswitch/scripts
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/scripts/ovs-check-dead-ifs $(1)/usr/share/openvswitch/scripts
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/scripts/ovs-ctl $(1)/usr/share/openvswitch/scripts
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/openvswitch/scripts/ovs-save $(1)/usr/share/openvswitch/scripts
endef

$(eval $(call BuildPackage,openvswitch-ipsec))
$(eval $(call BuildPackage,openvswitch-common))
$(eval $(call BuildPackage,openvswitch-switch))
$(eval $(call KernelPackage,openvswitch))

