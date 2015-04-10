# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.10 = .49
LINUX_VERSION-3.14 = .37

LINUX_KERNEL_MD5SUM-3.10.49 = 9774e12764e740d49c80eda77d0ef3eb
LINUX_KERNEL_MD5SUM-3.14.29 = a745f70181b573a34579d685ca16370e
LINUX_KERNEL_MD5SUM-3.14.37 = 43abcb454054c53fb07296e84119edc5

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
