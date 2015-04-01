BOARDNAME:=edge500

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

# edge500-specific packages:
DEFAULT_PACKAGES += \
	kmod-igb \
	igb-eeprom \
	kmod-dsa-velocloud \
	kmod-i2c-i801 \

define Target/Description
	Build firmware images for the Edge 500 and similar boards
endef

