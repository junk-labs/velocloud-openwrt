BOARDNAME:=VMware

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# VMware packages:
DEFAULT_PACKAGES += \
	kmod-pcnet32 \
	kmod-e1000 \
	kmod-e1000e \
	kmod-vmxnet3 \


define Target/Description
	Build firmware images for VMware images.
endef

