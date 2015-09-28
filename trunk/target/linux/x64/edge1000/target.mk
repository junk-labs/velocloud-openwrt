BOARDNAME:=edge500

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# edge500-specific packages:
DEFAULT_PACKAGES += \
	kmod-igb \
	kmod-dsa-core \
	kmod-ixgbe \
	kmod-e1000e \
	kmod-hwmon-core \
	kmod-hwmon-coretemp \
	kmod-hwmon-w83627ehf \
	igb-eeprom \

# Enable QAT for this platform
DEFAULT_PACKAGES += \
	kmod-intel-qat \
	intel-qat \

define Target/Description
	Build firmware images for the Edge 500 and similar boards
endef

