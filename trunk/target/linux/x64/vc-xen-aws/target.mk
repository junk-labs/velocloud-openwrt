BOARDNAME:=Xen/AWS-EC2

FEATURES:=ext4 pci usb display

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# Xen/AWS packages:
DEFAULT_PACKAGES += \
	kmod-xen-fs \
	kmod-xen-evtchn \
	kmod-xen-netdev \
	kmod-hyperv-balloon \
	kmod-hyperv-net-vsc \
	kmod-hyperv-util \
	kmod-hyperv-storage \
	kmod-igbvf \
	kmod-intel-i40evf \
	kmod-intel-ixgbevf \
	kmod-fs-vfat \
	kmod-fs-isofs \
	kmod-scsi-cdrom \
	kmod-nls-cp437 \
	kmod-nls-cp850 \
	kmod-nls-iso8859-1 \
	kmod-nls-iso8859-15 \


define Target/Description
	Build firmware images for Xen, AWS/EC2 and Hyper-V images.
endef

