BOARDNAME:=KVM Guest

FEATURES:=ext4 pci usb
# Common packages:
DEFAULT_PACKAGES += \
	openssh-server \
	openssh-client \
	openssl-util \


define Target/Description
	Build firmware images for VMware images.
endef

