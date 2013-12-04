#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=grub
PKG_VERSION:=2.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_MD5SUM:=e927540b6eda8b024fb0391eeaa4091c

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

# overwrite to install grub into different paths;
# if set, no installation into default image;
# used for Velocloud usb installer;

GRUB2_TARGET_DIR = $(KERNEL_BUILD_DIR)/inst.grub

define Package/grub2-target
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=disc
  TITLE:=grub2 tools for target
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86
endef

define Package/grub2-target/description
    grub2 tools for target
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status}; ./autogen.sh );
	$(call Build/Configure/Default, --disable-werror --disable-nls)
endef

define Package/grub2-target/install-mods
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/grub-core/moddep.lst \
		$(PKG_BUILD_DIR)/grub-core/*.mod \
		$(1)
endef

define Package/grub2-target/install-imgs
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/grub-core/*.img $(1)
endef

define Package/grub2-target/install-bins
	$(INSTALL_DIR) $(1)
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/grub-bin2h \
		$(PKG_BUILD_DIR)/grub-bios-setup \
		$(PKG_BUILD_DIR)/grub-editenv \
		$(PKG_BUILD_DIR)/grub-fstest \
		$(PKG_BUILD_DIR)/grub-install \
		$(PKG_BUILD_DIR)/grub-kbdcomp \
		$(PKG_BUILD_DIR)/grub-menulst2cfg \
		$(PKG_BUILD_DIR)/grub-mkconfig \
		$(PKG_BUILD_DIR)/grub-mkconfig_lib \
		$(PKG_BUILD_DIR)/grub-mkimage \
		$(PKG_BUILD_DIR)/grub-mklayout \
		$(PKG_BUILD_DIR)/grub-mknetdir \
		$(PKG_BUILD_DIR)/grub-mkpasswd-pbkdf2 \
		$(PKG_BUILD_DIR)/grub-mkrelpath \
		$(PKG_BUILD_DIR)/grub-mkrescue \
		$(PKG_BUILD_DIR)/grub-mkstandalone \
		$(PKG_BUILD_DIR)/grub-ofpathname \
		$(PKG_BUILD_DIR)/grub-probe \
		$(PKG_BUILD_DIR)/grub-reboot \
		$(PKG_BUILD_DIR)/grub-script-check \
		$(PKG_BUILD_DIR)/grub-set-default \
		$(PKG_BUILD_DIR)/grub-shell \
		$(PKG_BUILD_DIR)/grub-shell-tester \
		$(1)
endef

# install into package, or special path;

ifeq ($(GRUB2_TARGET_DIR),)
    define Package/grub2-target/install
	$(call Package/grub2-target/install-bins, $(1)/usr/bin)
	$(call Package/grub2-target/install-imgs, $(1)/boot/grub)
	$(call Package/grub2-target/install-mods, $(1)/boot/grub)
    endef
else
    define Package/grub2-target/install
	$(call Package/grub2-target/install-bins, $(GRUB2_TARGET_DIR)/bin)
	$(call Package/grub2-target/install-imgs, $(GRUB2_TARGET_DIR)/boot/grub)
	$(call Package/grub2-target/install-mods, $(GRUB2_TARGET_DIR)/boot/grub)
    endef
endif

$(eval $(call BuildPackage,grub2-target))
