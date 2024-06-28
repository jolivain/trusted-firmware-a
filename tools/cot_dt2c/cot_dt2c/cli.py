#
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from pathlib import Path
from cot_dt2c.cot_dt2c import generate_main

import click

@click.group()
@click.version_option()
def cli():
    pass

@cli.command()
@click.argument("inputfile", type=click.Path(dir_okay=True))
@click.argument("outputfile", type=click.Path(dir_okay=True))
def convert_to_c(inputfile, outputfile):
    generate_main(inputfile, outputfile)
