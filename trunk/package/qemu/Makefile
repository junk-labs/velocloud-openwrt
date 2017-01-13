# Copyright (C) 2014 Velocloud Inc
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=qemu
PKG_VERSION:=2.7.1
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://wiki.qemu-project.org/download/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=a315bc51ed443a08d2cf1416d76b9ab4

include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

define Package/qemu
  SECTION:=base
  CATEGORY:=VeloCloud
  VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
  TITLE:=QEMU for virtual machine guests.
  URL:=http://velocloud.net
  DEPENDS:=@USE_EGLIBC $(CXX_DEPENDS) +zlib +libuuid +libcurl +libncurses +glib2 +libusb-1.0 +libgpg-error +libgcrypt
endef

define Package/qemu/description
  QEMU for virtual machine guests.
endef

# configure qemu;

CONFIGURE_VARS += AS="$(TARGET_CROSS)as"
MAKE_FLAGS += AS="$(TARGET_CROSS)as"
CONFIGURE_ARGS = \
	--prefix=$(CONFIGURE_PREFIX) \
	--cross-prefix="$(TARGET_CROSS)" \
	--target-list=x86_64-softmmu \
	--host-cc="$(HOSTCC)" \
	--disable-debug-info \
	--disable-brlapi \
	--disable-sdl \
	--disable-xen \
	--disable-vnc \
	--disable-user \
        --disable-bzip2 \
        --disable-lzo \
        --disable-gnutls \
        --disable-nettle \
	--enable-system \
	--enable-kvm \
	--enable-vhost-net

define Build/Configure
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/qemu/install-bins
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/qemu-bridge-helper \
		$(PKG_BUILD_DIR)/qemu-ga \
		$(PKG_BUILD_DIR)/qemu-img \
		$(PKG_BUILD_DIR)/qemu-io \
		$(PKG_BUILD_DIR)/qemu-nbd \
		$(1)
endef

define Package/qemu/install-target
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/$(2)/$(3) \
		$(1)
endef

define Package/qemu/install-bios
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/pc-bios/*.bin \
		$(1)
endef

define Package/qemu/install
	$(call Package/qemu/install-bins, $(1)/usr/bin)
	$(call Package/qemu/install-target, $(1)/usr/bin,x86_64-softmmu,qemu-system-x86_64)
	$(call Package/qemu/install-bios, $(1)/usr/share/qemu)
endef

$(eval $(call BuildPackage,qemu))
