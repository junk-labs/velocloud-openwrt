BOARDNAME:=pw-dev

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
	kmod-r8169 \
	kmod-dsa-velocloud \

define Target/Description
	Build firmware images for Portwell Dev Board
endef

