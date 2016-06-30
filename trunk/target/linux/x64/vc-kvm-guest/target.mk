BOARDNAME:=KVM Guest

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# vc-kvm-guest specific packages:
DEFAULT_PACKAGES += \
	kmod-e1000 \
	kmod-virtio-net \
	kmod-virtio-random \
	kmod-igbvf \
	kmod-intel-ixgbevf \
	kmod-fs-vfat \
	kmod-fs-isofs \
	kmod-scsi-cdrom \
	kmod-nls-cp437 \
	kmod-nls-cp850 \
	kmod-nls-iso8859-1 \
	kmod-nls-iso8859-15 \


define Target/Description
	Build firmware images for KVM guest images.
endef

