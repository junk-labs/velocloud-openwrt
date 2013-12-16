# Makefile for building OpenWrt

# *Optional* local settings, instead of command line:
-include local.mk

# name of all supported target systems;
OPENWRT_CONFIG = openwrt.config
OPENWRT_VC_VERSION = 1
OPENWRT_TSYS = \
	sb-itc \
	pw-dev

# name can be changed to pull a specific branch;
OPENWRT_NAME ?= trunk
OPENWRT_SVN = svn://svn.openwrt.org/openwrt/$(OPENWRT_NAME)

# common shared download directory;
OPENWRT_DL ?= /eng/openwrt/dl

# force a certain rev;
OPENWRT_REV ?= 38900

# root of openwrt tree;
OPENWRT_ROOT = $(OPENWRT_NAME)

# auto-detect # of cores
NCPU ?= $(shell grep -c ^processor /proc/cpuinfo)

# nothing to do for default;
nothing:
	@echo "nothing default to do, on" $(NCPU) cores

# --------------------------------------------------
# for whoever pulls specific or latest openwrt tree;
# most people do not need this;

# checkout openwrt specific version;

.PHONY: checkout
openwrt-checkout:
	svn co $(OPENWRT_SVN)/@$(OPENWRT_REV) $(OPENWRT_ROOT)

# checkout openwrt latest;

.PHONY: checkout-head
openwrt-checkout-head:
	svn co $(OPENWRT_SVN)/@HEAD $(OPENWRT_ROOT)

# update openwrt to top-of-tree;

.PHONY: update
openwrt-update:
	svn update $(OPENWRT_ROOT)

# configure all feeds;
# there must be a trunk/feeds.conf file;

OPENWRT_FEED_SCRIPT = $(OPENWRT_ROOT)/scripts/feeds

.PHONY: feeds-all
openwrt-feeds: $(OPENWRT_ROOT)/feeds.conf
	$(OPENWRT_FEED_SCRIPT) update -a
	$(OPENWRT_FEED_SCRIPT) install -a

# -------------------------------
# make shared download dir;

define DownloadDir
	@if [ x"$(OPENWRT_DL)" != x"" ]; then \
		if [ ! -d $(OPENWRT_DL) ]; then \
			echo Error: OPENWRT_DL=$(OPENWRT_DL) is not a directory; \
			false; \
		elif [ -e $(OPENWRT_ROOT)/dl -a ! -h $(OPENWRT_ROOT)/dl ]; then \
			echo Warning: $(OPENWRT_ROOT)/dl already exists and not a symbolic link.; \
		else \
			rm -rf $(OPENWRT_ROOT)/dl; \
			ln -s $(OPENWRT_DL) $(OPENWRT_ROOT)/dl; \
		fi; \
	fi
endef

# ------------------------
# do manual configuration;
# use to create new trunk/.config, then save into config/...;

.PHONY: menuconfig
menuconfig:
	make -j1 -C $(OPENWRT_ROOT) menuconfig

.PHONY: kernel_menuconfig
kernel_menuconfig:
	make -j1 -C $(OPENWRT_ROOT) kernel_menuconfig

# -------------------------------
# actual build and clean targets;

# make board-specific config;
# target=x86 comes from $(OPENWRT_CONFIG)
# sub-target and profile is set here, based on build target;
# subtarget specific config come from trunk/target/linux/x86/xxx/target.mk;
# the profile trunk/target/linux/x86/xxx/profile/xxx.mk should only
# configure things like release or debug differences, not packages
# used for all platforms;

target_conf=$(subst .,_,$(subst -,_,$(subst /,_,$(1))))

define OpenwrtConfig
	@echo "Making subtarget:" $(call target_conf,$(1))
	@sed \
		-e '/CONFIG_VERSION_NICK=/d' \
		-e '/CONFIG_VERSION_NUMBER=/d' \
		-e '/CONFIG_TARGET_x86_/d' \
		-e '/CONFIG_X86_GRUB_SERIAL/d' \
		-e '/CONFIG_X86_GRUB_SERIAL_UNIT/d' \
		-e '/CONFIG_X86_GRUB_BOOTOPTS/d' \
		-e '$$ a\\n# target overwrites\n' \
		-e '$$ aCONFIG_TARGET_x86_$(call target_conf,$(1))=y' \
		-e '$$ aCONFIG_TARGET_x86_$(call target_conf,$(1))_Velocloud=y' \
		-e '$$ aCONFIG_VERSION_NICK="OpenWRT $(1)"' \
		-e '$$ aCONFIG_VERSION_NUMBER="$(OPENWRT_VC_VERSION)"' \
		-e '$$ a# CONFIG_X86_GRUB_SERIAL is not set' \
		-e '$$ a# CONFIG_X86_GRUB_SERIAL_UNIT is not set' \
		-e '$$ a# CONFIG_X86_GRUB_BOOTOPTS is not set' \
	$(OPENWRT_CONFIG) > $(OPENWRT_ROOT)/.config
	make -C $(OPENWRT_ROOT) defconfig
	make -C $(OPENWRT_ROOT) prereq
endef

# parallel build;
# if this fails:
#   cd trunk; make V=s

.PHONY: $(OPENWRT_TSYS)
$(OPENWRT_TSYS): $(OPENWRT_CONFIG)
	$(call DownloadDir)
	$(call OpenwrtConfig,$@)
	make -C $(OPENWRT_ROOT) -j $(NCPU)

# clean the build;
# cleans
#	$(OPENWRT_NAME)/bin/
#	$(OPENWRT_NAME)/build_dir/

.PHONY: clean
clean:
	make -C $(OPENWRT_ROOT) $@

# clean the build, staging and toolchain;
# clean
#	$(OPENWRT_NAME)/staging_dir/
#	$(OPENWRT_NAME)/toolchain/

.PHONY: dirclean
dirclean:
	make -C $(OPENWRT_ROOT) $@

# clean everything, including the downloaded pachages and feeds;
# careful: this also removes any non-saved config files;

.PHONY: distclean
distclean:
	make -C $(OPENWRT_ROOT) $@

