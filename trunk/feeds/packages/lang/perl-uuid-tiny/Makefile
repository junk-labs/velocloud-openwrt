
include $(TOPDIR)/rules.mk

PKG_NAME:=perl-uuid-tiny
PKG_VERSION:=1.04
PKG_RELEASE:=1
PKG_MD5SUM:=2d7c18711e64e0a64cc7c7fbb870947e

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/C/CA/CAUGUSTIN
PKG_SOURCE:=UUID-Tiny-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/UUID-Tiny-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-uuid-tiny
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=A collection of modules that parse HTML text documents
URL:=http://search.cpan.org/dist/UUID-Tiny/
DEPENDS:=perl
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-uuid-tiny/install
	$(call perlmod/Install,$(1),UUID)
endef


$(eval $(call BuildPackage,perl-uuid-tiny))
