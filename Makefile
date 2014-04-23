# Makefile for building OpenWrt

# *Optional* local settings, instead of command line:
-include local.mk

# x86 or x64
OPENWRT_ARCH = x64

# name of all supported target systems;
OPENWRT_CONFIG = openwrt.config
OPENWRT_VC_VERSION = 1
ifeq ($(OPENWRT_ARCH),x86)
OPENWRT_CPUARCH=i386
# Supported subtargets
OPENWRT_TSYS = \
	vc-vmdk \
	vc-xen-aws \
	sb-itc \
	pw-dev \

# Default subtargets
DEFAULT_OPENWRT_TSYS = \
	vc-vmdk \
	vc-xen-aws \
	sb-itc \

else
OPENWRT_CPUARCH=x86_64
# Supported subtargets
OPENWRT_TSYS = \
	vc-kvm-guest \
	vc-vmdk \
	vc-xen-aws \
	pw-dev \
	edge500 \

# Default subtargets
DEFAULT_OPENWRT_TSYS = \
	vc-kvm-guest \
	vc-vmdk \
	vc-xen-aws \
	pw-dev \
	edge500 \

endif


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

# default is to build all targets;
default: $(DEFAULT_OPENWRT_TSYS)

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

.PHONY: openwrt-fix-svn
# Git does not check in empty directories. Make sure that these are
# present before doing any svn updates in the tree.
openwrt-fix-svn:
	find trunk \
		-name build_dir -prune -o \
		-name staging_dir -prune -o \
		-type d -name .svn -print \
		-exec mkdir -p \
			{}/prop-base {}/props {}/text-base \
			{}/tmp/prop-base {}/tmp/props {}/tmp/text-base \;

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

define CopyFiles
	@echo "Copying config files for $(1)"
	@rm -rf $(OPENWRT_ROOT)/files
	@if [ -d $(OPENWRT_ROOT)/target/linux/$(OPENWRT_ARCH)/$(1)/files ]; then \
		mkdir -p $(OPENWRT_ROOT)/files ; \
		rsync -qax $(OPENWRT_ROOT)/target/linux/$(OPENWRT_ARCH)/$(1)/files/ $(OPENWRT_ROOT)/files/ ; \
	fi
endef

define OpenwrtConfig
	@echo "Making subtarget:" $(call target_conf,$(1))
	@sed \
		-e '/CONFIG_VERSION_NICK=/d' \
		-e '/CONFIG_VERSION_NUMBER=/d' \
		-e '/CONFIG_ARCH=/d' \
		-e '/CONFIG_ARCH_64BIT=/d' \
		-e '/CONFIG_i386=/d' \
		-e '/CONFIG_x86_64=/d' \
		-e '/CONFIG_CPU_TYPE=/d' \
		-e '/CONFIG_TARGET_x\(86\|64\)/d' \
		-e '/CONFIG_TARGET_BOARD/d' \
		-e '/CONFIG_TARGET_ROOTFS_PARTNAME/d' \
		-e '/CONFIG_X\(64\|86\)_/d' \
		-e '$$ a\\n# target overwrites\n' \
		-e '$$ aCONFIG_TARGET_$(OPENWRT_ARCH)=y' \
		-e '$$ aCONFIG_TARGET_$(OPENWRT_ARCH)_$(call target_conf,$(1))=y' \
		-e '$$ aCONFIG_TARGET_BOARD="$(OPENWRT_ARCH)"' \
		-e '$$ aCONFIG_$(OPENWRT_CPUARCH)=y' \
		-e '$$ aCONFIG_ARCH="$(OPENWRT_CPUARCH)"' \
		-e '$$ aCONFIG_VERSION_NICK="OpenWRT $(1)"' \
		-e '$$ aCONFIG_VERSION_NUMBER="$(OPENWRT_VC_VERSION)"' \
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
	$(call CopyFiles,$@)
	@rm -rf trunk/bin/$(OPENWRT_ARCH)-eglibc/root-$(OPENWRT_ARCH) trunk/bin/$(OPENWRT_ARCH)-eglibc/packages
	make -C $(OPENWRT_ROOT) -j $(NCPU)
	@rm -rf $(OPENWRT_ROOT)/files

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

