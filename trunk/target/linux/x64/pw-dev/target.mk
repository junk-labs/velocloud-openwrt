BOARDNAME:=pw-dev

FEATURES:=ext4 pci usb
# Common packages:
DEFAULT_PACKAGES += \
	bridge \
	ip \
	iputils-ping \
	iputils-ping6 \
	net-tools-arp \
	net-tools-ifconfig \
	net-tools-netstat \
	net-tools-route \
	openssh-server \
	openssh-client \
	openssl-util \


# pw-dev-specific packages:
DEFAULT_PACKAGES += \
	kmod-igb \
	igb-eeprom \
	kmod-dsa-velocloud \

define Target/Description
	Build firmware images for Portwell Dev Board
endef

