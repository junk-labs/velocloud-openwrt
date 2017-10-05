
include $(TOPDIR)/rules.mk

PKG_NAME:=perl-ipc-shareable
PKG_VERSION:=0.61
PKG_RELEASE:=1
PKG_MD5SUM:=8fb14d9e202651531ca5e5e5f1acd041

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/M/MS/MSOUTH
PKG_SOURCE:=IPC-Shareable-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/IPC-Shareable-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-ipc-shareable
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=A collection of modules that parse HTML text documents
URL:=http://search.cpan.org/dist/HTML-Parser/
DEPENDS:=perl
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-ipc-shareable/install
	$(call perlmod/Install,$(1),IPC)
endef


$(eval $(call BuildPackage,perl-ipc-shareable))
