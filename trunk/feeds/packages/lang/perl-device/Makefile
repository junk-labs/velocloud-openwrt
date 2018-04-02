
include $(TOPDIR)/rules.mk

PKG_NAME:=perl-device
PKG_VERSION:=0.09
PKG_RELEASE:=1
PKG_MD5SUM:=37fcd488e33a74756b85161393f32d5d

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/C/CO/COOK/
PKG_SOURCE:=Device-SerialPort-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Device-SerialPort-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-device
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=Serial Port IO in Perl
URL:=http://search.cpan.org/dist/Device-SerialPort/
DEPENDS:=perl
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-device/install
	$(call perlmod/Install,$(1),Device,)
endef


$(eval $(call BuildPackage,perl-device))
