BOARDNAME:=edge500

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# edge500-specific packages:
DEFAULT_PACKAGES += \
	kmod-igb \
	kmod-ixgbe \
	kmod-dsa-core \
	kmod-dsa-velocloud \
	kmod-hwmon-core \
	kmod-hwmon-coretemp \
	kmod-hwmon-w83627ehf \
	igb-eeprom \

define Target/Description
	Build firmware images for the Edge 500 and similar boards
endef

