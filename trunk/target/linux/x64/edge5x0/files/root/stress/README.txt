There are two versions of the stress loop test here.

For both, you need to run "setup.sh" once (which will cause a reboot),
after which the filesystem is now in stress-test mode forever.  (It's
best to do this while running from a USB key!)

Also, it's best to do the setup step BEFORE connecting all the network
cables; otherwise it can cause packet storms across the switch ports.

"stress_loop.sh" is the NEW loop setup (where SFPs are looped together,
as are the WAN ports, and the LAN switch ports are looped only to each
other).  See the script itself for the loop setup:

        SFP1 to SFP2
        GE1 to GE2
        LAN1 to LAN5
        LAN2 to LAN6
        LAN3 to LAN7
        LAN4 to LAN8

It's run as follows:
     "./stress_loop.sh [-d seconds] [-s]"
         -d is the duration (in seconds) of the test (how long it runs).
            The default value is 360000 (100 hours)
         -s means also stress the CPU by running a couple of CPU-loading
            processes ("openssl speed") in a loop while the test is running.


P.S.  "old_stress_loop.sh" expects the OLD loop setup (where SFPs were
connected to GE ports, and thus only supported Copper SFPs).  This is
the version you'll see documented in many places in Google Drive, until
they are all fixed up.
