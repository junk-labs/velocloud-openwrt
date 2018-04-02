# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.10 = .49
LINUX_VERSION-3.14 = .67

LINUX_KERNEL_MD5SUM-3.10.49 = 9774e12764e740d49c80eda77d0ef3eb
LINUX_KERNEL_MD5SUM-3.14.43 = 927f2343f298dfe531a8371f81356e53
LINUX_KERNEL_MD5SUM-3.14.56 = 32010dbb68e50b994bcf845635b616d5
LINUX_KERNEL_MD5SUM-3.14.67 = 98eee7939f4bc5354e45e4ed71b1ab7d

ifdef KERNEL_PATCHVER
  LINUX_VERSION:=$(KERNEL_PATCHVER)$(strip $(LINUX_VERSION-$(KERNEL_PATCHVER)))
endif

split_version=$(subst ., ,$(1))
merge_version=$(subst $(space),.,$(1))
KERNEL_BASE=$(firstword $(subst -, ,$(LINUX_VERSION)))
KERNEL=$(call merge_version,$(wordlist 1,2,$(call split_version,$(KERNEL_BASE))))
KERNEL_PATCHVER ?= $(KERNEL)

# disable the md5sum check for unknown kernel versions
LINUX_KERNEL_MD5SUM:=$(LINUX_KERNEL_MD5SUM-$(strip $(LINUX_VERSION)))
LINUX_KERNEL_MD5SUM?=x
