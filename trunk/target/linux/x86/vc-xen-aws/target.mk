BOARDNAME:=Xen/AWS-EC2

DEFAULT_PACKAGES += kmod-xen-fs kmod-xen-evtchn kmod-xen-netdev kmod-xen-kbddev
FEATURES:=ext4 pci usb display

define Target/Description
	Build firmware images for Xen and AWS/EC2 images.
endef

