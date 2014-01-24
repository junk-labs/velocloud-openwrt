BOARDNAME:=edge500

FEATURES:=ext4 pci usb
# Common packages:
DEFAULT_PACKAGES += \
	openssh-server \
	openssh-client \
	openssl-util \


# pw-dev-specific packages:
DEFAULT_PACKAGES += \
	kmod-intel-igb \
	igb-eeprom \
	kmod-dsa-velocloud \

define Target/Description
	Build firmware images for the Edge 500 and similar boards
endef

