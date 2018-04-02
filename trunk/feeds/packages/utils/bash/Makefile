#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bash
PKG_VERSION:=4.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bash
PKG_MD5SUM:=81348932d5da294953e15d4814c74dd1

include $(INCLUDE_DIR)/package.mk

define Package/bash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The GNU Bourne Again SHell
  DEPENDS:=+libncurses
  URL:=http://www.gnu.org/software/bash/
endef

define Package/bash/description
	Bash is an sh-compatible command language interpreter that executes
	commands read from the standard input or from a file. Bash also
	incorporates useful features from the Korn and C shells (ksh and csh).
endef


define Build/Configure
	$(call Build/Configure/Default, \
		--without-bash-malloc \
                --bindir=/bin \
		ac_cv_func_mmap_fixed_mapped=yes \
		ac_cv_func_strcoll_works=yes \
		ac_cv_func_working_mktime=yes \
		gt_cv_int_divbyzero_sigfpe=yes \
		bash_cv_func_sigsetjmp=present \
		bash_cv_getcwd_malloc=yes \
		bash_cv_job_control_missing=present \
		bash_cv_printf_a_format=yes \
		bash_cv_sys_named_pipes=present \
		bash_cv_ulimit_maxfds=yes \
		bash_cv_sys_siglist=yes \
		bash_cv_wexitstatus_offset=0 \
		bash_cv_under_sys_siglist=yes \
		bash_cv_unusable_rtsigs=no \
		bash_cv_wcontinued_broken=no \
		bash_cv_func_ctype_nonascii=no \
		bash_cv_dup2_broken=no \
		bash_cv_pgrp_pipe=no \
		bash_cv_opendir_not_robust=no \
		bash_cv_getenv_redef=yes \
		bash_cv_func_strcoll_broken=no \
		bash_cv_func_snprintf=yes \
		bash_cv_func_vsnprintf=yes \
		bash_cv_must_reinstall_sighandlers=no \
	)
endef


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/builtins LDFLAGS_FOR_BUILD= mkbuiltins
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		SHELL="/bin/bash" \
		all install
endef

define Package/bash/postinst
#!/bin/sh
grep bash $${IPKG_INSTROOT}/etc/shells || \
	echo "/bin/bash" >> $${IPKG_INSTROOT}/etc/shells
	echo "/bin/rbash" >> $${IPKG_INSTROOT}/etc/shells
endef

define Package/bash/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/bash $(1)/bin/
	ln -sf bash $(1)/bin/rbash
endef

$(eval $(call BuildPackage,bash))
