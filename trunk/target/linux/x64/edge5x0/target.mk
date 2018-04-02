BOARDNAME:=edge5x0

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# edge520/540-specific packages:
DEFAULT_PACKAGES += \
	dmi-tool \
	flashrom \
	kmod-dsa-core \
	kmod-dsa-velocloud \
	kmod-i2c-i801 \
	kmod-i2c-ismt \
	kmod-i2c-ltc4266 \
	kmod-hwmon-core \
	kmod-hwmon-coretemp \
	kmod-hwmon-w83627ehf \
	igb-eeprom \
	trousers \
	tpm-tools \

# Enable QAT for this platform
DEFAULT_PACKAGES += \
	kmod-intel-qat \
	intel-qat \

define Target/Description
	Build firmware images for the Edge 520/540 and similar boards
endef

