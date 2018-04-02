cd /root

# 0. **Optional** step 0 - Save off old ROM

./flashrom -p internal -r ./my-current.rom

# --
# Step 1. Flash new Boot rom:

./dmi-tool -u firmware/vc5x0-cb-01-bootorder.rom

# This should update the old boot flash, and show you what
# entries it's preserving. With this release (01.00.00.00),
# it seems to preserve the product serial number, UUID, and
# board serial number, but it reads garbage for the product
# name and # board version (both of which are new).

# The fields that are shown are:
# SN=VCExxxxxxxxxx, UUID_str=009fed51-565d-4fe0-b87f-f05daaf94c0c, BSN=xxxxxxxxxx, pname=EDGE520 bversion=1

# The key fields here are the product name and board version.
# 
#   pname = one of "EDGE520" or "EDGE540"  (without quotes)
#   bversion = one of "1" (rev A) or "2.0" (rev B)  (without quotes)
# 


# --
# Step 2. Flash the appropriate updated DMI info for product name and board version if needed:

./dmi-tool -w -p EDGE520 -v 1

# --
# Step 3. Confirm all the info is correct:

./dmi-tool -r

# Verify all of the fields, and then power-cycle.

