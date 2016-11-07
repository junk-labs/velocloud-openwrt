I. Setting up Build System
==========================

1. On a fresh Ubuntu 14.04 system, as root:

* apt-get update

2. Install the following packages (using apt-get install):

* git
* build-essential
* bison
* flex
* gawk
* zip and unzip
* gdisk
* gettext
* libneon27-gnutls
* libncurses5-dev libncurses5
* zlib1g-dev zlib1g
* libapr1 libaprutil1
* devscripts
* dh-make

3. To install subversion, you need to get a special set of packages for
subversion 1.6 from /eng/openwrt/svn-1.6 (there are three packages there).
scp them to some temporary directory, and execute:

  # dpkg -i *.deb  (the 3 files)
  # apt-mark hold subversion libsvn1    # to prevent upgrades



II. Checking out Sources
========================

* Create a working directory.
* Checkout sources from git@git.assembla.com:velocloud.openwrt.git
    % git clone git@git.assembla.com:velocloud.openwrt.git openwrt

III. Building the sources
=========================

* To build a single architecture (e.g. edge500), invoke

    % make edge500

This will take about an hour.
