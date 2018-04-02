BOARDNAME:=edge8x0

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# edge500-specific packages:
DEFAULT_PACKAGES += \
	kmod-dsa-core \
	kmod-hwmon-core \
	kmod-hwmon-coretemp \
	igb-eeprom \

# Enable QAT for this platform
DEFAULT_PACKAGES += \
	kmod-intel-qat \
	intel-qat \

define Target/Description
	Build firmware images for the Edge 800 and similar systems
endef

