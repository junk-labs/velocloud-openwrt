This is the combined contents of the "edge/progs/vlan" and "common/libs/marvell" directories
from the velocloud source tree, with a few mods:

* Macro magic to turn all the DBG_LOGs into syslog calls (which should be cleaned up!)
* Include paths fixed (no "marvell/*")

* Config file format changed to:

{
    "ports": [
        {"vlanIds": [1,2,64], "defaultVlanId": 1, "mode": "trunk",
         "switch": 0, "port": 0},
        {"vlanIds": [2,64], "defaultVlanId": 2, "mode": "trunk",
         "switch": 0, "port": 1},
        {"vlanIds": [1], "defaultVlanId": 1, "mode": "access",
         "switch": 0, "port": 2},
        {"vlanIds": [1], "defaultVlanId": 1, "mode": "access",
         "switch": 0, "port": 3}
    ]
}

If there are multiple switches (e.g. 520/540), there will be one array of
ports for the logical port numbers 0..7 (sw0..sw7), but the upper 4 will
have switch == 1 and port == 0..3.

To map these "switch IDs" and "ports" into the actual devices needed for
ioctls is (should be) straightforward, as long as the driver has a sane
data structure to hold both switches with indexes 0 and 1 (and maybe publish
whatever parameter is needed to access them to some sysfs entry somewhere).

Details TBD, of course, between Sandra, Jegadish and me.
