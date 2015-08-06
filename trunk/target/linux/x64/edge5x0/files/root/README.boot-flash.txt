cd /root

# 0. Optional step 0 - Save off old ROM

flashrom -p internal -r ./my-current.rom

# --
# Step 1. Flash new Boot rom:

dmi-tool -u firmware/vc5x0-cb-01-bootorder.rom

# This should update the old boot flash, and show you what
# entries it's preserving. With this release (01.00.00.00),
# it seems to preserve the product serial number, UUID, and
# board serial number, but it reads garbage for the product
# name and # board version (both of which are new).

# --
# Step 2. Flash the updated DMI info for product name and board version

dmi-tool -w -p EDGE520 -v 1

# version should be "1" for revA board, and "2.0" for revB board


# --
# Step 3. Confirm all the info is correct:

dmi-tool -r

# Verify all of the fields.

