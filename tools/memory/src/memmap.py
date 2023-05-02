#!/usr/bin/env python3
#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
from pathlib import Path

import click
from buildparser import TfaBuildParser
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
    help="The platform targeted for analysis (default is fvp).",
)
@click.option(
    "-b",
    "--build-type",
    default="release",
    help="The target build type (debug or release).",
)
@click.option(
    "-f",
    "--footprint",
    is_flag=True,
    help="Generate a high level view of memory usage by memory types.",
)
@click.option(
    "-t",
    "--tree",
    is_flag=True,
    help="Generate a hierarchical view of the modules, segments and sections.",
)
@click.option(
    "--depth",
    default=3,
    help="Generate a virtual address map of important TF symbols.",
)
@click.option(
    "-s",
    "--symbols",
    is_flag=True,
    help="Generate a map of important TF symbols.",
)
@click.option("-w", "--width", type=int, envvar="COLUMNS")
@click.option(
    "-d",
    is_flag=True,
    default=False,
    help="Display numbers in decimal base.",
)
def main(root, platform, build_type, footprint, tree, symbols, depth, width, d):
    build_path = Path(root) if root else Path("build/", platform, build_type)
    click.echo(f"build-path: {build_path.resolve()}")

    parser = TfaBuildParser(build_path)
    printer = TfaPrettyPrinter(columns=width, as_decimal=d)

    modules = parser.get_sorted_mod_names()

    if footprint or not (tree or symbols):
        printer.print_footprint(parser.get_mem_usage_dict())

    if tree:
        printer.print_mem_tree(
            parser.get_mem_tree_as_dict(), modules, depth=depth
        )

    if symbols:
        expr = (
            r"(.*)(TEXT|BSS|RODATA|STACKS|_OPS|PMF|XLAT|GOT|FCONF"
            r"|R.M)(.*)(START|END)__$"
        )
        printer.print_symbol_table(parser.get_sorted_symbols(expr), modules)


if __name__ == "__main__":
    main()
