BOARDNAME:=Xen/AWS-EC2

FEATURES:=ext4 pci usb display

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# Xen/AWS packages:
DEFAULT_PACKAGES += \
	kmod-xen-fs \
	kmod-xen-evtchn \
	kmod-xen-netdev \


define Target/Description
	Build firmware images for Xen and AWS/EC2 images.
endef

