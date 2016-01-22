#!/usr/bin/python

import glob
import json
import time
import sys

args = sys.argv
if len(args) == 1:
    files = glob.glob('/tmp/USB/USB*_periodic.txt')
else:
    files = ['/tmp/USB/%s_periodic.txt' % a for a in args[1:]]

total = 0.0
count = 0
for filename in files:
    try:
        with open(filename, 'r') as fp:
            json_value = json.load(fp)
            total = total + float(json_value['SigPercentage'])
            count = count + 1
    except:
        pass

if count:
    total = total / (count * 20.0)

# round up to next integer
total = int(total + 0.5)
print total
