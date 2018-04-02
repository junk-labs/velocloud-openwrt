Testing various wireless configurations
---------------------------------------

* Insert the flash drive.  

* Create a temporary directory to mount this drive, and mount it:

   # mkdir /tmp/mnt
   # mount /dev/sdb1 /tmp/mnt

* Copy all the files under <flashdrive>/config/ to /etc/config/ (these
  are all called "wireless.<something>"), and unmount the flash drive:

   # cp /tmp/mnt/config/wireless.* /etc/config/
   # umount /tmp/mnt

* The files are:

   wireless.emissions    - Turn all radios off

   wireless.baseline0-24 - Turn only radio0 on, transmitting on 2.4 GHz
   wireless.baseline0-5 - Turn only radio0 on, transmitting on 5 GHz
   wireless.baseline1-24 - Turn only radio1 on, transmitting on 2.4 GHz
   wireless.baseline1-5 - Turn only radio1 on, transmitting on 5 GHz

   wireless.colo-24-24 - Turn on both radios, both transmitting on 2.4 GHz
                         (different channels - 1 and 11)
   wireless.colo-5-5 - Turn on both radios, both transmitting on 5 GHz
                         (different channels - 149 and 157)
   wireless.colo-24-5 - Turn on both radios, one transmitting on 2.4 GHz
                         and one on 5 GHz.

* For each test, do the following commands ("#" is the shell prompt):

   # cd /etc/config
   # cp wireless.<configuration> wireless
   # wifi

 This will restart wi-fi with the new configuration.

To verify that the wi-fi is broadcasting correctly, try connecting to the
corresponding SSIDs in each case:

    radio 0 - "vc-wifi-fcc-n"  (WPA password == "vcsecret")
    radio 1 - "vc-wifi-fcc-ac"  (WPA password == "vcsecret")


