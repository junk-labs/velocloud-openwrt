BOARDNAME:=pw-dev

FEATURES:=ext4 pci usb

# Common packages:
include $(PLATFORM_SUBDIR)/../velocloud/common-packages.mk

# pw-dev-specific packages:
DEFAULT_PACKAGES += \
	kmod-igb \
	kmod-dsa-core \
	kmod-dsa-velocloud \
	kmod-hwmon-core \
	kmod-hwmon-coretemp \
	kmod-hwmon-w83627ehf \
	igb-eeprom \

# Enable QAT for this platform
DEFAULT_PACKAGES += \
	kmod-intel-qat \
	intel-qat \

define Target/Description
	Build firmware images for Portwell Dev Board
endef

