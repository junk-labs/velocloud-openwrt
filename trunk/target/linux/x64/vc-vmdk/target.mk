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


define Target/Description
	Build firmware images for VMware images.
endef

