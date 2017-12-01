# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

export PATH=$(TARGET_PATH):/sbin

GRUB2_MODULES = biosdisk boot chain configfile ext2 linux ls part_msdos part_gpt reboot vga acpi smbios vc
GRUB2_MODULES_ISO = biosdisk boot chain configfile iso9660 linux ls part_msdos reboot vga
GRUB_TERMINALS =
GRUB_SERIAL_CONFIG =
GRUB_TERMINAL_CONFIG =
GRUB_CONSOLE_CMDLINE =

USE_ATKBD = kvm_guest

QEMU_IMG = STAGING_DIR_HOST=$(STAGING_DIR_HOST) sh ./qemu-img.sh

ifneq ($(strip $(foreach subtarget,$(USE_ATKBD),$(CONFIG_TARGET_x64_$(subtarget)))),)
  GRUB2_MODULES += at_keyboard
  GRUB2_MODULES_ISO += at_keyboard
endif

ifneq ($(CONFIG_GRUB_CONSOLE),)
  GRUB_CONSOLE_CMDLINE += console=tty0
  GRUB_TERMINALS += console
endif

GRUB_SERIAL:=$(call qstrip,$(CONFIG_GRUB_SERIAL))

GRUB_UNIT:=0
ifneq ($(CONFIG_GRUB_SERIAL_UNIT),)
  GRUB_UNIT:=$(call qstrip,$(CONFIG_GRUB_SERIAL_UNIT))
endif

ifneq ($(GRUB_SERIAL),)
  GRUB2_MODULES += pci extcmd terminfo serial
  GRUB_CONSOLE_CMDLINE += console=$(GRUB_SERIAL),$(CONFIG_GRUB_BAUDRATE)n8
  GRUB_SERIAL_PCH:=$(subst PCH,pch,$(subst tty,,$(GRUB_SERIAL)))
ifeq ($(shell echo $(GRUB_SERIAL_PCH) | grep pch),)
  GRUB_SERIAL_CONFIG := serial --unit=$(GRUB_UNIT) --speed=$(CONFIG_GRUB_BAUDRATE) --word=8 --parity=no --stop=1
  GRUB_TERMINALS += serial
else
  GRUB_SERIAL_CONFIG := serial $(GRUB_SERIAL_PCH) --speed=$(CONFIG_GRUB_BAUDRATE)
  GRUB_TERMINALS += serial_$(GRUB_SERIAL_PCH)
endif
endif

ifneq ($(GRUB_TERMINALS),)
  GRUB_TERMINAL_CONFIG := terminal_input $(GRUB_TERMINALS); terminal_output $(GRUB_TERMINALS)
endif

ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))

GRUB_TIMEOUT:=$(call qstrip,$(CONFIG_GRUB_TIMEOUT))

ifneq ($(CONFIG_GRUB_IMAGES),)

  BOOTOPTS:=$(call qstrip,$(CONFIG_GRUB_BOOTOPTS)) panic_timeout=5

  define Image/cmdline/ext4
    root=$(ROOTPART) rootfstype=ext4 rootwait
  endef

ifneq ($(CONFIG_EC2_IMAGES),)
  define Image/cmdline/ext4-ec2
    root=$(call qstrip,$(CONFIG_TARGET_ROOTFS_EC2_PARTNAME)) rootfstype=ext4 rootwait
  endef
endif

  define Image/cmdline/jffs2-64k
    block2mtd.block2mtd=$(ROOTPART),65536,rootfs root=/dev/mtdblock0 rootfstype=jffs2 rootwait
  endef

  define Image/cmdline/jffs2-128k
    block2mtd.block2mtd=$(ROOTPART),131072,rootfs root=/dev/mtdblock0 rootfstype=jffs2 rootwait
  endef

  define Image/cmdline/squashfs
    block2mtd.block2mtd=$(ROOTPART),65536,rootfs root=/dev/mtdblock0 rootfstype=squashfs rootwait
  endef

  # The root=@PARTUUID@ below is really for when we have GPTs in the
  # installed images. For now, the installer will strip it out.
  define Image/cmdline/velocloud
    root=@PARTUUID@ rootfstype=ext4 rootwait
  endef

  define Image/Build/grub2
	# left here because the image builder doesnt need these
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	grub-mkimage \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/core.img \
		-O i386-pc \
		-c ./grub-early.cfg \
		$(GRUB2_MODULES)
	$(CP) $(STAGING_DIR_HOST)/lib/grub/i386-pc/*.img $(KDIR)/grub2/
	echo '(hd0) $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img' > $(KDIR)/grub2/device.map
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#$(strip $(call Image/cmdline/$(1)) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		./grub.cfg > $(KDIR)/root.grub/boot/grub/grub.cfg
	PADDING="$(CONFIG_TARGET_IMAGES_PAD)" PATH="$(TARGET_PATH)" ./gen_image_generic.sh \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/root.grub \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(KDIR)/root.$(1) \
		1024
	grub-bios-setup \
		--device-map="$(KDIR)/grub2/device.map" \
		-d "$(KDIR)/grub2" \
		-r "hd0,msdos1" \
		"$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img"
	$(call Image/Build/grub/$(1))
  endef
endif

ROOTDELAY=10

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build/iso
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	grub-mkimage \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/eltorito.img \
		-O i386-pc \
		-c ./grub-early.cfg \
		$(GRUB2_MODULES_ISO)
	cat \
		$(STAGING_DIR_HOST)/lib/grub/i386-pc/cdboot.img \
		$(KDIR)/grub2/eltorito.img \
		> $(KDIR)/root.grub/boot/grub/eltorito.img
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#root=/dev/sr0 rootfstype=iso9660 rootwait $(strip $(call Image/cmdline/$(1)) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		./grub-iso.cfg > $(KDIR)/root.grub/boot/grub/grub.cfg
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	mkisofs -R -b boot/grub/eltorito.img -no-emul-boot -boot-info-table \
		-o $(KDIR)/root.iso $(KDIR)/root.grub $(TARGET_DIR)
endef

ifneq ($(CONFIG_VDI_IMAGES),)
  define Image/Build/vdi
    # left here because the image builder doesnt need these
    ifeq ($(1),ext4)
		rm $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi || true
		$(QEMU_IMG) convert -f raw -O vdi \
			$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
			$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi
		# XXX: VBoxManage insists on setting perms to 0600
		chmod 0644 $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi
    endif
  endef
endif

define Image/Build/gzip
	gzip -f9 $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img
	gzip -f9 $(BIN_DIR)/$(IMG_PREFIX)-rootfs-$(1).img
endef

ifneq ($(CONFIG_TARGET_IMAGES_GZIP),)
  define Image/Build/gzip/ext4
	$(call Image/Build/gzip,ext4)
  endef
  ifneq ($(CONFIG_TARGET_IMAGES_PAD),)
    define Image/Build/gzip/squashfs
	$(call Image/Build/gzip,squashfs)
    endef
    define Image/Build/gzip/jffs2-64k
	$(call Image/Build/gzip,jffs2-64k)
    endef
    define Image/Build/gzip/jffs2-128k
	$(call Image/Build/gzip,jffs2-128k)
    endef
  endif
endif

define Image/BuildKernel
	$(CP) $(KDIR)/bzImage $(BIN_DIR)/$(IMG_PREFIX)-vmlinuz
	$(CP) $(KDIR)/bzImage $(TARGET_DIR)/vmlinuz
	$(CP) $(KDIR)/bzImage.kdump $(TARGET_DIR)/vmlinuz.kdump
endef

define Image/Build/Profile/ar525w
	cp $(KDIR)/bzImage $(KDIR)/bzImage.tmp
	$(SCRIPT_DIR)/pad_image $(1) $(KDIR)/bzImage.tmp $(KDIR)/root.$(1) 32
	$(STAGING_DIR_HOST)/bin/airlink -b 1 $(KDIR)/bzImage.tmp $(KDIR)/root.$(1) \
		$(BIN_DIR)/$(IMG_PREFIX)-$(1)-ar525w.img
	$(STAGING_DIR_HOST)/bin/airlink -e -b 1 $(KDIR)/bzImage.tmp $(KDIR)/root.$(1) \
		$(BIN_DIR)/$(IMG_PREFIX)-$(1)-ar525w-web.img
endef

define Image/Build/Profile/sitecom
	cp $(KDIR)/bzImage $(KDIR)/bzImage.tmp
	#32k config data + 20 bytes header + 2 bytes checksum after kernel image
	$(SCRIPT_DIR)/pad_image $(1) $(KDIR)/bzImage.tmp $(KDIR)/root.$(1) 32790
	$(TOPDIR)/target/linux/rdc/image/mkimg_sitecom.pl $(KDIR)/bzImage.tmp > $(KDIR)/tmp.img
	cat $(KDIR)/root.$(1) >> $(KDIR)/tmp.img
	cp $(KDIR)/tmp.img $(BIN_DIR)/$(IMG_PREFIX)-$(1)-sitecom.img
	rm $(KDIR)/tmp.img $(KDIR)/bzImage.tmp
endef

# velocloud common image generator;

# $(1): install dir
# $(2): in
# $(3): out

define Image/Build/velocloud/grubcfg
	$(INSTALL_DIR) $(KDIR)/$(1)/boot/grub
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#$(strip $(call Image/cmdline/velocloud) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		$(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/grub/$(2) \
		> $(KDIR)/$(1)/boot/grub/$(3)
endef

# $1: root fstype;
# $2: image name;

define Image/Build/velocloud/gpt
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	grub-mkimage \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/core.img \
		-O i386-pc \
		-c ./grub-gpt-early.cfg \
		$(GRUB2_MODULES)
	$(CP) $(STAGING_DIR_HOST)/lib/grub/i386-pc/*.img $(KDIR)/grub2/
	$(call Image/Build/velocloud/grubcfg,inst.grub,grub-velocloud.cfg,grub-velocloud.cfg)
	$(INSTALL_DIR) $(KDIR)/firmware
	if [ -d $(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/firmware ]; then \
		$(CP) -HL $(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/firmware $(KDIR)/ || true; \
	fi
	$(INSTALL_DIR) $(KDIR)/mfgtest
	if [ -d $(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/mfgtest ]; then \
		$(RSYNC) -avH $(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/mfgtest/ $(KDIR)/mfgtest/ || true; \
	fi
	chmod u+x ./gen_image_velocloud.sh
	GRUB2_MODULES="$(GRUB2_MODULES)" \
	PATH="$(TARGET_PATH)" \
	STAGING_DIR_HOST=$(STAGING_DIR_HOST) \
		./gen_image_velocloud.sh \
		$(BIN_DIR)/$(2) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/root.grub \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(KDIR)/root.$(1) \
		$(CONFIG_TARGET_ROOTFS_INSTSIZE) \
		$(TARGET_DIR) \
		$(KDIR)/root.grub \
		$(KDIR)/inst.grub \
		$(KDIR)/firmware $(KDIR)/mfgtest
	echo '(hd0) $(BIN_DIR)/$(2)' > $(KDIR)/grub2/device.map
	grub-bios-setup \
		--device-map="$(KDIR)/grub2/device.map" \
		-d "$(KDIR)/grub2" \
		-r "hd0,gpt2" \
		"$(BIN_DIR)/$(2)"
	$(call Image/Build/grub/$(1))
endef

define Image/Build/velocloud/updatable-gpt
	chmod u+x ./gen_image_vc_virtual.sh
	PATH="$(TARGET_PATH)" \
	STAGING_DIR_HOST=$(STAGING_DIR_HOST) \
	INCLUDE_DIR=$(INCLUDE_DIR) \
		./gen_image_vc_virtual.sh \
		$(BIN_DIR)/$(2) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/inst.grub \
		$(CONFIG_TARGET_ROOTFS_INSTSIZE) $(TARGET_DIR) \
		$(CONFIG_TARGET_VDISK_SIZE)
	grub-mkimage \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/inst.grub/boot/grub/core.img \
		-O i386-pc \
		-c ./grub-gpt-early.cfg \
		$(GRUB2_MODULES) test echo loadenv
	echo '(hd0) $(BIN_DIR)/$(2)' > $(KDIR)/inst.grub/boot/grub/device.map
	grub-bios-setup \
		--device-map="$(KDIR)/inst.grub/boot/grub/device.map" \
		-d "$(KDIR)/inst.grub/boot/grub" \
		-r "hd0,gpt2" \
		"$(BIN_DIR)/$(2)"
	$(call Image/Build/grub/$(1))
endef

ifneq ($(CONFIG_VELOCLOUD_INSTALLER),)
  define Image/Build/velocloud-installer
	echo "Building VeloCloud Installer Image."
	$(call Image/Build/velocloud/grubcfg,root.grub,grub-installer.cfg,grub.cfg)
	$(call Image/Build/velocloud/gpt,$(1),$(IMG_PREFIX)-installer-usb-$(1).img)
  endef
endif

ifneq ($(CONFIG_VELOCLOUD_UPDATABLE_IMAGE),)
  define Image/Build/velocloud-updatable-image
	echo "Building raw VeloCloud VM Image."
	$(call Image/Build/velocloud/grubcfg,inst.grub,grub-velocloud.cfg,grub-velocloud.cfg)
	$(call Image/Build/velocloud/updatable-gpt,$(1),$(IMG_PREFIX)-updatable-$(1).img)
  endef
endif

ifneq ($(CONFIG_EC2_IMAGES),)
  ifneq ($(CONFIG_VELOCLOUD_UPDATABLE_IMAGE),)
    define Image/Build/vc-ec2-updatable
	# Build AWS (EC2) Updatable Image
	tar -C $(BIN_DIR) \
		-cvzSf $(BIN_DIR)/$(IMG_PREFIX)-updatable-ec2-$(1).tgz \
		$(IMG_PREFIX)-updatable-$(1).img
    endef
  endif

  define Image/Build/vc-ec2
	# Build single-partition (paravirt) EC2 Image
	E2SIZE=$(shell echo $$(($(CONFIG_TARGET_ROOTFS_PARTSIZE)*1024)))
	mkdir -p $(TARGET_DIR)/boot/grub
	$(CP) $(KDIR)/bzImage $(TARGET_DIR)/boot/vmlinuz
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#$(strip $(call Image/cmdline/$(1)-ec2) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		$(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/grub/menu.lst \
		  > $(TARGET_DIR)/boot/grub/menu.lst

	$(STAGING_DIR_HOST)/bin/genext2fs -U -B $(CONFIG_TARGET_EXT4_BLOCKSIZE) -b $(E2SIZE) -N $(CONFIG_TARGET_EXT4_MAXINODE) -m $(CONFIG_TARGET_EXT4_RESERVED_PCT) -d $(TARGET_DIR)/ $(BIN_DIR)/$(IMG_PREFIX)-ec2-$(1).img
	$(STAGING_DIR_HOST)/bin/tune2fs -O extents,uninit_bg,dir_index $(BIN_DIR)/$(IMG_PREFIX)-ec2-$(1).img
	$(STAGING_DIR_HOST)/bin/e2fsck -fy $(BIN_DIR)/$(IMG_PREFIX)-ec2-$(1).img
  endef
endif

ifneq ($(CONFIG_VMDK_IMAGES),)
  define Image/Build/vmdk
	echo "Building VeloCloud Updatable Virtual Image."
	rm $(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).vmdk || true
	$(QEMU_IMG) convert -f raw -O vmdk \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).vmdk
  endef

  ifneq ($(CONFIG_OVA_IMAGES),)
    define Image/Build/ova
	chmod u+x ./gen_ova.sh
	STAGING_DIR_HOST=$(STAGING_DIR_HOST) \
	IMG_PREFIX=$(IMG_PREFIX) \
	ARCH=$(ARCH) \
	./gen_ova.sh \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).vmdk \
		$(TOPDIR)/target/linux/$(BOARD)/$(SUBTARGET)/ovf/VeloCloud-Edge.ovf
    endef
  endif
endif

ifneq ($(CONFIG_QCOW2_IMAGES),)
  define Image/Build/qcow2
	echo "Building VeloCloud Updatable Virtual Image."
	rm $(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).qcow2 || true
	$(QEMU_IMG) convert -f raw -O qcow2 \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).qcow2
  endef
endif

ifneq ($(CONFIG_VHD_IMAGES),)
  define Image/Build/vhd
	echo "Building VeloCloud Updatable Virtual Image."
	rm $(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).vhd || true
	$(QEMU_IMG) convert -f raw -O vpc -o subformat=fixed,force_size \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).vhd
  endef
endif

define Image/Build/Profile/bifferboard
	$(TOPDIR)/target/linux/rdc/image/mkimg_bifferboard.py $(KDIR)/bzImage $(KDIR)/root.$(1) \
		$(BIN_DIR)/$(IMG_PREFIX)-$(1)-bifferboard.img
endef

define Image/Prepare
	rm -rf $(KDIR)/bzImage
	$(CP) $(LINUX_DIR)/arch/x86/boot/bzImage $(KDIR)/bzImage
	$(call Image/Prepare/bootscript)
	$(call Image/Prepare/grub2)
endef

define Image/Build/Initramfs
	$(CP) $(KDIR)/bzImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-ramfs.bzImage
endef

define Image/Build
	$(call Image/Build/$(1))
	$(call Image/Build/bootscript,$(1))
  ifneq ($(1),iso)
	echo ENV_CONFIG_NO_INSTALLER_IMAGE=$(ENV_CONFIG_NO_INSTALLER_IMAGE)
    ifeq ($(ENV_CONFIG_NO_INSTALLER_IMAGE),)
	$(call Image/Build/velocloud-installer,$(1))
    endif
	echo ENV_CONFIG_NO_COMBINED_IMAGE=$(ENV_CONFIG_NO_COMBINED_IMAGE)
    ifeq ($(ENV_CONFIG_NO_COMBINED_IMAGE),)
	$(call Image/Build/grub2,$(1))
    endif
	$(call Image/Build/velocloud-updatable-image,$(1))
	$(call Image/Build/vdi,$(1))
	$(call Image/Build/vmdk,$(1))
	$(call Image/Build/ova,$(1))
	$(call Image/Build/qcow2,$(1))
	$(call Image/Build/vhd,$(1))
	$(call Image/Build/vc-ec2-updatable,$(1))
	$(call Image/Build/vc-ec2,$(1))
	# not needed any more:
	rm -f $(BIN_DIR)/$(IMG_PREFIX)-updatable-$(1).img
	$(CP) $(KDIR)/root.$(1) $(BIN_DIR)/$(IMG_PREFIX)-rootfs-$(1).img
  else
	$(CP) $(KDIR)/root.iso $(BIN_DIR)/$(IMG_PREFIX).iso
  endif
	$(CP) $(KDIR)/bzImage $(BIN_DIR)/$(IMG_PREFIX)-vmlinuz
	$(call Image/Build/gzip/$(1))
	$(call Image/Build/Profile/$(PROFILE),$(1))
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	$(call Image/Build/Initramfs)
endif
endef

$(eval $(call BuildImage))
