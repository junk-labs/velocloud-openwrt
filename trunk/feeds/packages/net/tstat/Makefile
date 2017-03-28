include $(TOPDIR)/rules.mk

PKG_NAME:=tstat
PKG_VERSION:=3.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://tstat.polito.it/download
PKG_MD5SUM:=2460381664b5abbd4d4c466b46092726

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=libpcap libpthread

define Package/tstat
  SECTION:=net
  CATEGORY:=Network
  TITLE:=TCP STatistic and Analysis Tool from POLITO
  URL:=http://tstat.tlc.polito.it
  DEPENDS:=+libpcap +libpthread +zlib +libstdcpp
  MAINTAINER:=Tivadar Szemethy <tivadar.szemethy@netvisor.hu>
endef

define Package/tstat/description
 Tstat is a passive sniffer able to provide several insight on the traffic
 patterns at both the network and the transport levels.
endef

define Package/tstat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tstat/tstat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tstat))
