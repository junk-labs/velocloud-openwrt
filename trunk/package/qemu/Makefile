# Copyright (C) 2014 Velocloud Inc
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=qemu
PKG_VERSION:=1.7.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://wiki.qemu-project.org/download/qemu-1.7.0.tar.bz2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=32893941d40d052a5e649efcf06aca06

include $(INCLUDE_DIR)/package.mk

define Package/qemu
  SECTION:=base
  CATEGORY:=VeloCloud
  VERSION:=1.0-$(PKG_RELEASE)
  TITLE:=QEMU for virtual machine guests.
  URL:=http://velocloud.net
  DEPENDS:=+USE_EGLIBC +libz +libuuid +libcurl +libncurses +glib2 +libgthread-2.0
endef

define Package/qemu/description
  QEMU for virtual machine guests.
endef

# configure qemu;

CONFIGURE_ARGS = \
	--prefix=$(CONFIGURE_PREFIX) \
	--cross-prefix=$(GNU_TARGET_NAME) \
	--target-list=i386-softmmu,x86_64-softmmu \
	--host-cc=gcc \
	--disable-brlapi \
	--disable-sdl \
	--disable-xen \
	--disable-vnc \
	--disable-user \
	--enable-system \
	--enable-kvm \
	--enable-vhost-net

TARGET_CONFIGURE_OPTS += AS="$(TARGET_CC) -c $(TARGET_ASFLAGS) -x assembler"

define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/qemu/install-bins
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/qemu-bridge-helper \
		$(PKG_BUILD_DIR)/qemu-ga \
		$(PKG_BUILD_DIR)/qemu-img \
		$(PKG_BUILD_DIR)/qemu-io \
		$(PKG_BUILD_DIR)/qemu-nbd \
		$(1)
endef

define Package/qemu/install-target
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/$(2)/$(3) \
		$(1)
endef

define Package/qemu/install-bios
	$(INSTALL_DIR) $(1)/usr/share/qemu/
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/pc-bios/*.bin \
		$(1)
endef

define Package/qemu/install
	$(call Package/qemu/install-bins, $(1))
	$(call Package/qemu/install-target, $(1),i386-softmmu,qemu-system-i386)
	$(call Package/qemu/install-target, $(1),x86_64-softmmu,qemu-system-x86_64)
	$(call Package/qemu/install-bios, $(1))
endef

$(eval $(call BuildPackage,qemu))
