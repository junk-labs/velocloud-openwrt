BOARDNAME:=pw-dev

FEATURES:=ext4 pci usb

ADDL_KMOD_PACKAGES:= \
	kmod-crypto-aes \
	kmod-crypto-crc32c \
	kmod-crypto-deflate \
	kmod-crypto-des \
	kmod-crypto-md4 \
	kmod-crypto-md5 \
	kmod-crypto-hash \
	kmod-ipt-queue \
	kmod-lib-crc32c \
	kmod-lib-lzo \
	libatomicops \
	grub2 \
	luci-lib-core_source \
	luci-lib-nixio-notls \
	iptables-mod-ulog \
	kmod-crypto-core \
	kmod-crypto-manager \
	mkelfimage \
	miniupndd \
	uhttpd-mod-tls-cyassl \


DEFAULT_PACKAGES+=\
	base-files \
	busybox \
	dnsmasq \
	-dropbear \
	-dropbearconvert \
	firewall \
	hotplug2 \
	iptables \
	mtd \
	netifd \
	opkg \
	qos-scripts \
	swconfig \


define Target/Description
	Build firmware images for Portwell Dev Board
endef

