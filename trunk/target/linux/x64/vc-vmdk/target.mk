BOARDNAME:=VMware

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# VMware packages:
DEFAULT_PACKAGES += \
	open-vm-tools \
	kmod-pcnet32 \
	kmod-e1000 \
	kmod-e1000e \
	kmod-vmxnet3 \
	kmod-igbvf \
	kmod-intel-ixgbevf \
	kmod-fs-vfat \
	kmod-fs-isofs \
	kmod-nls-cp437 \
	kmod-nls-cp850 \
	kmod-nls-iso8859-1 \
	kmod-nls-iso8859-15 \


define Target/Description
	Build firmware images for VMware images.
endef

