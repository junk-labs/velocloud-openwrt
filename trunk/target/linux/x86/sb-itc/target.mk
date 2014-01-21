BOARDNAME:=sb-itc

FEATURES:=ext4 pci usb
# Common packages:
DEFAULT_PACKAGES += \
	openssh-server \
	openssh-client \
	openssl-util \


# SB-ITC-specific packages:
DEFAULT_PACKAGES += \
	kmod-intel-igb \
	igb-eeprom \
	kmod-r8169 \
	kmod-dsa-velocloud \


define Target/Description
	Build firmware images for SB-ITC boards (dev & protos).
endef
