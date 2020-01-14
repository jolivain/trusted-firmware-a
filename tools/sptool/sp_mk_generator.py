#!/usr/bin/python3
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
This script is invoked by Make system and generates secure partition makefile.
It expects platform provided secure partition layout file and parses it to get
Image, Resource description and UUID for a given SP. This information is then
populated into generated make file.

param1: Generated mk file "sp_gen.mk"
param2: json file containing platform specific information
param3: plat out directory

Generated "sp_gen.mk" file contains triplet of following information for each
Secure Partition entry
    FDT_SOURCES +=  sp1.dts
    SPTOOL_ARGS += -i sp1.bin:sp1.dtb -o sp1.pkg
    FIP_ARGS += --blob uuid=XXXXX-XXX...,file=sp1.pkg

"""

import getopt
import json
import os
import sys

with open(sys.argv[2],'r') as in_file:
    data = json.load(in_file)
dir_name = os.path.dirname(sys.argv[2])
gen_file = sys.argv[1]
out_dir = sys.argv[3][2:]
dtb_dir = out_dir + "/fdts/"
print(dtb_dir)

with open(gen_file, 'w') as out_file:
    for key in data.keys():

        """
        Append FDT_SOURCES
        """
        dts = dir_name + "/" + data[key]['dts']
        dtb = dtb_dir + data[key]['dts'][:-1] + "b"
        out_file.write("FDT_SOURCES += " + dts + "\n")

        """
        Update SPTOOL_ARGS
        """
        dst = out_dir + "/" + key + ".pkg"
        src = [ dir_name + "/" + data[key]['image'] , dtb  ]
        out_file.write("SPTOOL_ARGS += -i " + ":".join(src) + " -o " + dst + "\n")

        """
        Append FIP_ARGS
        """
        uuid = data[key]['uuid']
        out_file.write("FIP_ARGS += --blob uuid=" + uuid + ",file=" + dst + "\n")
        out_file.write("\n")
