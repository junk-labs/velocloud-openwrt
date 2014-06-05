BOARDNAME:=Xen/AWS-EC2

FEATURES:=ext4 pci usb display
# Common packages:
DEFAULT_PACKAGES += \
	openssh-server \
	openssh-client \
	openssl-util \

# Xen/AWS packages:
DEFAULT_PACKAGES += \
	kmod-xen-fs \
	kmod-xen-evtchn \
	kmod-xen-netdev \


define Target/Description
	Build firmware images for Xen and AWS/EC2 images.
endef

