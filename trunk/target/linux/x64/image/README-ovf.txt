Generating OVF/OVA Packages for VMDKs
=====================================

Overview:
--------

There's a fair amount of black magic here:

* We are trying to avoid using ovftool itself to package OVFs, since it's
  a big, bloated beast (and an opaque binary), and we don't need 99% of it.

  Instead, we generate an OVF file from a hand-deployed and configured VM
  on ESX 5.5, and carefully edit it so that:

  **  we give the networks and interfaces symbolic names, instead of
      whatever happened to be bound to that interface when it was set up.
      (In ESX, when you add an interface, you have to bind it to a network,
       even if you plan to not connect it.)
  **  we replace the disk name and size with "@@" placeholders, to be
      replaced (below).

* We generate the VMDK normally.

* Then we run a script called "gen_ova.sh", that:

  ** Uses "vmware-vdiskmanager" (binary checked into source tree, and
     installed into staging_dir/host/{bin,lib}) to generate a compressed,
     "streaming" format of vdisk.
  ** Substitute the name and size of the disk into a copy of the OVF
     template above
  ** Create a manifest file with the sha1sum of the OVF and the vmdk.
  ** Create an OVA file, which just a tar of the OVF, manifest, and disk.

That seems straightforward enough.

vmware-vdiskmanager:
-------------------

Unfortunately, this tool isn't available as source. It's only available
as part of a larger tarball of a "VDDK" (VMware DDK) that has some
libraries, headers, and this tool, which links against some of those
libraries.

So we have a simple tools/vmware-vdiskmanager/Makefile that just unpacks
the tarball (which we check in), and exports the required files to the
staging_dir/host/bin and .../lib directories.

The tricky part is that is that this tool has some hidden dependencies
because of dynamic loading:

   * It loads libvixDiskLib.so, libvixDiskLibVim.so and libgvmomi.so
     dynamically, using "normal" dynamic library loading mechanisms. We
     have to put these in staging_dir/host/lib, and use LD_LIBRARY_PATH
     whenever we launch vmware-vdiskmanager.

   * It also dynamically loads libssl.so.0.9.8 and libcrypto.so.0.9.8
     using a bizarre, home-grown, multi-openssl-version "abstraction"
     layer, that involves searching for specific versions of openssl in
     some weird, non-obvious places (like /usr/lib/vmware), and it DOES
     NOT honor the LD_LIBRARY_PATH variable.

     It only works, almost by accident, if these libraries are in the
     "bin" directory itself (a Windows-like hangover), so we have to
     install these there.

Also, one final thing to remember is that in order for files in "bin"
to be properly exported into the ImageBuilder, they must have execute
permissions, so we have to be careful to either chmod it, or use
$(INSTALL_BIN) to copy it.

Editing the OVF file
--------------------

Be EXTREMELY CAREFUL if you edit the .ovf template by hand. VMware's
systems (ESX, Fusion, Workstation, etc) are extremely finicky about the
OVF that they accept, and more often than not, they'll either die with
a mysterious error (if you're lucky), or just silently corrupt stuff (if
you're unlucky).

   (As a data point: when I used ovftool to export a Fusion VM, and
    tried to import it into ESX, the import worked silently, and created
    a VM disk of the right size, but with all zero pages.)

The best way to edit this file is to:

* Import an existing OVA into ESX 5.5.

* Set up the Network interfaces so that they all point to different ESX
  networks (this makes the subsequent hand-edits easier).

* Make whatever configuration changes you want to make (adding, deleting
  or changing devices)

* Run the resultant VM and make sure it works for you.

* Now shut down the VM, and export the OVF as a folder of files. You are
  only interested in the OVF.

Now, for the hand-edits:

* In the References/File node near the head, replace the ovf:href (disk
  name) with @DISKNAME@, and the ovf:size with "@DISKSIZE@". If you see
  an ovf:chunksize attribute, delete it.

* In the DiskSection/Disk node, also near the head, remove any attribute
  called ovf:populatedSize.

* Now look at the NetworkSection. You'll see the 4 (or however many you
  have in there) networks, hopefully with distinct names (if you've
  followed my instruction above).

  It'll look something like:

	<Network ovf:name="VM Network">
	  <Description>The VM Network network</Description>
	</Network>

  These will be correlated to device nodes further down in the file,
  looking like <Item>/...<rasd:Connection>VM Network</rasd:Connection>...

  ** Do a global replace of the name ("VM Network"), so that the first
     one is called "LAN", and the second, third, fourth, etc. are called
     WAN1, WAN2, WAN3, etc. So now they should look like

	<Network ovf:name="LAN">
	  <Description>The LAN network</Description>
	</Network>

     (and also in the <Item> that referred to it below - that should also
      say "VmxNet3 ethernet adapter on &quot;LAN&quot;")



After editing, DIFF THE OLD AND NEW FILE (can't stress this enough), and
make sure that each change is something you deliberately introduced.

Finally, build an OVA with this new OVF template in the toolchain (or the
imagebuilder), and make sure that when you deploy it, it shows the right
thing.
