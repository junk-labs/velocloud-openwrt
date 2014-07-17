BOARDNAME:=Xen/AWS-EC2

FEATURES:=ext4 pci usb display
# Common packages:
DEFAULT_PACKAGES += \
	bridge \
	ip \
	net-tools-arp \
	net-tools-ifconfig \
	net-tools-netstat \
	net-tools-route \
	openssh-server \
	openssh-client \
	openssl-util \

# Xen/AWS packages:
DEFAULT_PACKAGES += \
	kmod-xen-fs \
	kmod-xen-evtchn \
	kmod-xen-netdev \
	kmod-xen-kbddev \


define Target/Description
	Build firmware images for Xen and AWS/EC2 images.
endef

