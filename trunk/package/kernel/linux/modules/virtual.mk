#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

VIRTUAL_MENU:=Virtualization Support

define KernelPackage/virtio
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=VirtIO settings for KVM guest
  DEPENDS:=@TARGET_x86_kvm_guest||TARGET_x64_vc_kvm_guest
  DEFAULT:=y if (TARGET_x64_vc_kvm_guest)
  KCONFIG:= \
	CONFIG_VIRTIO=y \
	CONFIG_VIRTIO_BLK=y \
	CONFIG_VIRTIO_CONSOLE=y \
	CONFIG_VIRTIO_PCI=y \
	CONFIG_SCSI_VIRTIO=y
endef

define KernelPackage/virtio/description
 Kernel settings for KVM guests with Virtio
endef

$(eval $(call KernelPackage,virtio))


define KernelPackage/virtio-balloon
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=VirtIO balloon driver
  DEPENDS:=@TARGET_x86_kvm_guest||TARGET_x64_vc_kvm_guest
  KCONFIG:=CONFIG_VIRTIO_BALLOON
  FILES:=$(LINUX_DIR)/drivers/virtio/virtio_balloon.ko
  AUTOLOAD:=$(call AutoLoad,06,virtio-balloon)
endef

define KernelPackage/virtio-balloon/description
 Kernel module for VirtIO memory ballooning support
endef

$(eval $(call KernelPackage,virtio-balloon))


define KernelPackage/virtio-net
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=VirtIO network driver
  DEPENDS:=@TARGET_x86_kvm_guest||TARGET_x64_vc_kvm_guest
  DEFAULT:=y if (TARGET_x64_vc_kvm_guest)
  KCONFIG:=CONFIG_VIRTIO_NET
  FILES:=$(LINUX_DIR)/drivers/net/virtio_net.ko
  AUTOLOAD:=$(call AutoLoad,50,virtio_net)
endef

define KernelPackage/virtio-net/description
 Kernel module for the VirtIO paravirtualized network device
endef

$(eval $(call KernelPackage,virtio-net))


define KernelPackage/virtio-random
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=VirtIO Random Number Generator support
  DEPENDS:=@TARGET_x86_kvm_guest||TARGET_x64_vc_kvm_guest
  DEFAULT:=y if (TARGET_x64_vc_kvm_guest)
  KCONFIG:=CONFIG_HW_RANDOM_VIRTIO
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/virtio-rng.ko
  AUTOLOAD:=$(call AutoLoad,09,virtio-rng)
endef

define KernelPackage/virtio-random/description
 Kernel module for the VirtIO Random Number Generator
endef

$(eval $(call KernelPackage,virtio-random))

define KernelPackage/xen
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen Kernel Settings
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:= \
  	CONFIG_PARAVIRT=y \
	CONFIG_PARAVIRT_DEBUG=y \
  	CONFIG_HYPERVISOR_GUEST=y \
  	CONFIG_XEN=y \
  	CONFIG_XEN_BLKDEV_FRONTEND=y \
  	CONFIG_INPUT_XEN_KBDDEV_FRONTEND=y \
  	CONFIG_HVC_XEN=y \
  	CONFIG_HVC_XEN_FRONTEND=y \
  	CONFIG_TCG_XEN=y \
  	CONFIG_XEN_BALLOON=y \
  	CONFIG_XEN_SCRUB_PAGES=y \
  	CONFIG_XEN_BACKEND=y \
  	CONFIG_XEN_SYS_HYPERVISOR=y \
  	CONFIG_XEN_GNTDEV \
  	CONFIG_XEN_GRANT_DEV_ALLOC \
  	CONFIG_XEN_PCIDEV_BACKEND \
  	CONFIG_XEN_ACPI_PROCESSOR
endef

define KernelPackage/xen/description
 Kernel settings for Xen
endef

$(eval $(call KernelPackage,xen))

define KernelPackage/xen-fs
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen filesystem
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:= \
  	CONFIG_XENFS \
  	CONFIG_XEN_COMPAT_XENFS=y
  FILES:= \
	$(LINUX_DIR)/drivers/xen/xenfs/xenfs.ko \
	$(LINUX_DIR)/drivers/xen/xen-privcmd.ko
  AUTOLOAD:=$(call AutoLoad,05,xenfs xen-privcmd)
endef

define KernelPackage/xen-fs/description
 Kernel module for the Xen filesystem
endef

$(eval $(call KernelPackage,xen-fs))


define KernelPackage/xen-evtchn
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen event channels
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:=CONFIG_XEN_DEV_EVTCHN
  FILES:=$(LINUX_DIR)/drivers/xen/xen-evtchn.ko
  AUTOLOAD:=$(call AutoLoad,06,xen-evtchn)
endef

define KernelPackage/xen-evtchn/description
 Kernel module for the /dev/xen/evtchn device
endef

$(eval $(call KernelPackage,xen-evtchn))

define KernelPackage/xen-fbdev
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen virtual frame buffer
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws +kmod-fb
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:= \
  	CONFIG_XEN_FBDEV_FRONTEND \
  	CONFIG_FB_DEFERRED_IO=y \
  	CONFIG_FB_SYS_COPYAREA \
  	CONFIG_FB_SYS_FILLRECT \
  	CONFIG_FB_SYS_FOPS \
  	CONFIG_FB_SYS_IMAGEBLIT \
  	CONFIG_FIRMWARE_EDID=n
  FILES:= \
  	$(LINUX_DIR)/drivers/video/xen-fbfront.ko \
  	$(LINUX_DIR)/drivers/video/syscopyarea.ko \
  	$(LINUX_DIR)/drivers/video/sysfillrect.ko \
  	$(LINUX_DIR)/drivers/video/fb_sys_fops.ko \
  	$(LINUX_DIR)/drivers/video/sysimgblt.ko
  AUTOLOAD:=$(call AutoLoad,07, \
  	fb \
  	syscopyarea \
  	sysfillrect \
  	fb_sys_fops \
  	sysimgblt \
  	xen-fbfront \
  )
endef

define KernelPackage/xen-fbdev/description
 Kernel module for the Xen virtual frame buffer
endef

$(eval $(call KernelPackage,xen-fbdev))


define KernelPackage/xen-kbddev
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen virtual keyboard and mouse
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:=CONFIG_XEN_KBDDEV_FRONTEND
  FILES:=$(LINUX_DIR)/drivers/input/misc/xen-kbdfront.ko
  AUTOLOAD:=$(call AutoLoad,08,xen-kbdfront)
endef

define KernelPackage/xen-kbddev/description
 Kernel module for the Xen virtual keyboard and mouse
endef

$(eval $(call KernelPackage,xen-kbddev))


define KernelPackage/xen-netdev
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen network device frontend
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:=CONFIG_XEN_NETDEV_FRONTEND
  FILES:=$(LINUX_DIR)/drivers/net/xen-netfront.ko
  AUTOLOAD:=$(call AutoLoad,09,xen-netfront)
endef

define KernelPackage/xen-netdev/description
 Kernel module for the Xen network device frontend
endef

$(eval $(call KernelPackage,xen-netdev))


define KernelPackage/xen-pcidev
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Xen PCI device frontend
  DEPENDS:=@TARGET_x86_xen_domu||TARGET_x86_vc_xen_aws||TARGET_x64_vc_xen_aws
  DEFAULT:=y if (TARGET_x86_xen_domu || TARGET_x86_vc_xen_aws || TARGET_x64_vc_xen_aws)
  KCONFIG:=CONFIG_XEN_PCIDEV_FRONTEND
  FILES:=$(LINUX_DIR)/drivers/pci/xen-pcifront.ko
  AUTOLOAD:=$(call AutoLoad,10,xen-pcifront)
endef

define KernelPackage/xen-pcidev/description
 Kernel module for the Xen network device frontend
endef

$(eval $(call KernelPackage,xen-pcidev))

#
# Hyper-V Drives depends on x86 or x86_64.
#
define KernelPackage/hyperv-balloon
  SUBMENU:=$(VIRTUAL_MENU)
  DEPENDS:=@TARGET_x64_vc_xen_aws
  TITLE:=Microsoft Hyper-V Balloon Driver
  KCONFIG:= \
    CONFIG_HYPERV_BALLOON \
    CONFIG_HYPERVISOR_GUEST=y \
    CONFIG_PARAVIRT=n \
    CONFIG_HYPERV=y
  FILES:=$(LINUX_DIR)/drivers/hv/hv_balloon.ko
  AUTOLOAD:=$(call AutoLoad,06,hv_balloon)
endef

define KernelPackage/hyperv-balloon/description
  Microsofot Hyper-V balloon driver.
endef

$(eval $(call KernelPackage,hyperv-balloon))

define KernelPackage/hyperv-net-vsc
  SUBMENU:=$(VIRTUAL_MENU)
  DEPENDS:=@TARGET_x64_vc_xen_aws
  TITLE:=Microsoft Hyper-V Network Driver
  KCONFIG:= \
    CONFIG_HYPERV_NET \
    CONFIG_HYPERVISOR_GUEST=y \
    CONFIG_PARAVIRT=n \
    CONFIG_HYPERV=y
  FILES:=$(LINUX_DIR)/drivers/net/hyperv/hv_netvsc.ko
  AUTOLOAD:=$(call AutoLoad,35,hv_netvsc)
endef

define KernelPackage/hyperv-net-vsc/description
  Microsoft Hyper-V Network Driver
endef

$(eval $(call KernelPackage,hyperv-net-vsc))

define KernelPackage/hyperv-util
  SUBMENU:=$(VIRTUAL_MENU)
  DEPENDS:=@TARGET_x64_vc_xen_aws
  TITLE:=Microsoft Hyper-V Utility Driver
  KCONFIG:= \
    CONFIG_HYPERV_UTILS \
    CONFIG_HYPERVISOR_GUEST=y \
    CONFIG_PARAVIRT=n \
    CONFIG_HYPERV=y
  FILES:=$(LINUX_DIR)/drivers/hv/hv_utils.ko
  AUTOLOAD:=$(call AutoLoad,10,hv_utils)
endef

define KernelPackage/hyperv-util/description
  Microsoft Hyper-V Utility Driver
endef

$(eval $(call KernelPackage,hyperv-util))

#
# Hyper-V Storage Drive needs to be in kernel rather than module to load the root fs.
#
define KernelPackage/hyperv-storage
  SUBMENU:=$(VIRTUAL_MENU)
  DEPENDS:=@TARGET_x64_vc_xen_aws +kmod-scsi-core
  TITLE:=Microsoft Hyper-V Storage Driver
  KCONFIG:= \
    CONFIG_HYPERV_STORAGE=y \
    CONFIG_HYPERVISOR_GUEST=y \
    CONFIG_PARAVIRT=n \
    CONFIG_HYPERV=y
  FILES:=$(LINUX_DIR)/drivers/scsi/hv_storvsc.ko
  AUTOLOAD:=$(call AutoLoad,40,hv_storvsc)
endef

define KernelPackage/hyperv-storage/description
  Microsoft Hyper-V Storage Driver
endef

$(eval $(call KernelPackage,hyperv-storage))


define KernelPackage/kvm
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Kernel KVM support for Intel/AMD
  DEPENDS:=@TARGET_x86||TARGET_x64
  KCONFIG:=CONFIG_VIRTUALIZATION=y \
	CONFIG_IOMMU_SUPPORT=y \
	CONFIG_AMD_IOMMU=y \
	CONFIG_AMD_IOMMU_V2=y \
	CONFIG_INTEL_IOMMU=y \
	CONFIG_HYPERVISOR_GUEST=y \
	CONFIG_HIGH_RES_TIMERS=y \
	CONFIG_KVM_DEVICE_ASSIGNMENT=y \
	CONFIG_KVM \
	CONFIG_KVM_INTEL \
	CONFIG_KVM_AMD \
	CONFIG_VHOST_NET
  FILES:=$(LINUX_DIR)/arch/x86/kvm/kvm.ko \
	$(LINUX_DIR)/arch/x86/kvm/kvm-intel.ko \
	$(LINUX_DIR)/arch/x86/kvm/kvm-amd.ko \
	$(LINUX_DIR)/drivers/net/tun.ko \
	$(LINUX_DIR)/drivers/vhost/vhost.ko \
	$(LINUX_DIR)/drivers/vhost/vhost_net.ko
endef

define KernelPackage/kvm/description
 Kernel modules for KVM support
endef

$(eval $(call KernelPackage,kvm))

define KernelPackage/vc_kvm_guest
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Kernel KVM Guest support
  DEPENDS:=@TARGET_x64_vc_kvm_guest
  DEFAULT:=y if (TARGET_x64_vc_kvm_guest)
  KCONFIG:= \
	CONFIG_KVM_GUEST=y \
	CONFIG_HYPERVISOR_GUEST=y \
	CONFIG_PARAVIRT=y \
	CONFIG_PARAVIRT_DEBUG=y \
	CONFIG_PARAVIRT_CLOCK=y
endef

define KernelPackage/vc_kvm_guest/description
 Kernel modules for supporting qemu-kvm-1.5x on OpenStack Havana
endef

$(eval $(call KernelPackage,vc_kvm_guest))

define KernelPackage/vmware
  SUBMENU:=$(VIRTUAL_MENU)
  TITLE:=Support for VMware Virtual Machines
  DEPENDS:=@TARGET_x64_vc_vmdk
  DEFAULT:=y if (TARGET_x64_vc_vmdk)
  KCONFIG:= \
	CONFIG_VMWARE_PVSCSI=y
endef

define KernelPackage/vmware/description
 Kernel settings for VMware guests
endef

$(eval $(call KernelPackage,vmware))


define KernelPackage/vmxnet3
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=VMware VMXNET3 ethernet driver 
  DEPENDS:=@(TARGET_x64_vc_vmdk&&PCI_SUPPORT)
  DEFAULT:=y if (TARGET_x64_vc_vmdk)
  KCONFIG:=CONFIG_VMXNET3
  FILES:=$(LINUX_DIR)/drivers/net/vmxnet3/vmxnet3.ko
  AUTOLOAD:=$(call AutoLoad,35,vmxnet3)
endef

define KernelPackage/vmxnet3/description
 Kernel modules for VMware VMXNET3 ethernet adapters.
endef

$(eval $(call KernelPackage,vmxnet3))
