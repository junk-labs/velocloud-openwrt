Integrating the Mocana sources
==============================

The way I have integrated the Mocana build into the OpenWrt build is as
follows:

* I did _not_ edit src/common/moptions.h (or moptions_custom.h) as the
  documentation describes. Instead, I have a custom top-level Makefile
  (courtesy of Mocana themselves) that passes the right definitions using
  -D options to each of the 3 builds (ipsec modules, ike and loadConfig).

* To work around an issue with the 3.8.13 kernel having some header files
  with the same names and paths as Mocana's files (e.g. "crypto/des.h"),
  we pass in a bunch of "-include .../crypto/...h" directives (also provided
  by Mocana).

* The Makefile here is a typical OpenWrt Makefile that creates two packages:
  one for the ipsec modules, and one for the "other stuff" (so far, only
  ike and loadConfig).

The Mocana sources are just the "zip" file straight from Mocana, including
their weird zip password ("mocana").  The OpenWrt makefile just unpacks it
into the build directory, copies over the top-level Makefile mentioned above,
and launches the Compile step.

Picking up a new build
======================

When Mocana gives us new drops, we can just drop the new .zip file, and
update some entries in the OpenWrt makefile (assuming they keep the same
pattern of distribution: an "mss_date_version.zip" file).  Just update
the PKG_REV macro to match the new version, and check in the new .zip and
delete the old one (not needed any more).

We might have to tweak Makefile.mocana for future releases (esp if the
Linux kernel issue above is solved - we can then remove the -include
directives).
