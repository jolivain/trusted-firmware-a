#!/usr/bin/env python3
#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
from pathlib import Path

import click
from tfaparser import MemParser
from elftools.elf.elffile import ELFFile
from printer import TfaPrettyPrinter


@click.command()
@click.option(
    "-r",
    "--root",
    type=Path,
    default=None,
    help="Root containing build output.",
)
@click.option(
    "-p",
    "--platform",
    default="fvp",
    help="The platform targeted for analysis.",
)
@click.option(
    "-b",
    "--build-type",
    default="release",
    help="The target build type i.e. debug or release.",
)
@click.option(
    "-f",
    "--footprint",
    is_flag=True,
    help="Prints the total memory footprint (ROM/SRAM) used by TF-A.",
)
@click.option(
    "-t",
    "--table",
    is_flag=True,
    help="Prints module level memory information (address range, memory size).",
)
@click.option(
    "-m", "--map", is_flag=True, help="Prints the builds virtual memory map."
)
def main(root, platform, build_type, footprint, table, map):
    build_path = Path(root) if root else Path("build/", platform, build_type)

    parser = MemParser()
    printer = TfaPrettyPrinter()

    click.echo(f"build-path: {build_path.resolve()}")

    for _m in build_path.glob("**/*.elf"):
        with open(_m, "rb") as f:
            elf_file = ELFFile(f)
            with open(_m.with_suffix(".map"), "r") as map_file:
                parser.add_module(elf_file, map_file)

    if not parser.modules:
        click.echo(f"No ELF files found in: {build_path.absolute()}.")
        exit(1)

    if table:
        printer.print_tab_mem_usage(parser.modules)
    if footprint:
        printer.print_mem_footprint(
            parser.get_mem_alloc(), parser.get_mem_total_free()
        )
    if map:
        printer.print_virt_mem_map(
            parser.get_sorted_sections(), parser.get_sorted_mod_names()
        )


if __name__ == "__main__":
    main()
