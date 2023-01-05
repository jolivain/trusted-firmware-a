#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
import re

import click
from prettytable import PrettyTable


class TfaPrettyPrinter:
    def __init__(self, term_size: int = 120):
        self.term_size = term_size

    @staticmethod
    def to_hex(val, padding=10):
        return f"{val:#0{padding}x}"

    @staticmethod
    def get_mem_info_str(name, size, free):
        return f"Total {name} memory: {size} bytes [free = {free}]"

    @staticmethod
    def get_virt_map_row_str(
        section_name, rel_pos, columns, width=None, padding=None, border=False
    ):
        empty_col = "{:{}{}}"
        sec_row = f"+{section_name:-^{width-1}}+"
        sep, fill = ("+", "-") if border else ("|", "")

        sec_row_l = empty_col.format(sep, fill + "<", width) * rel_pos
        sec_row_r = empty_col.format(sep, fill + ">", width) * (
            columns - rel_pos - 1
        )

        return padding + sec_row_l + sec_row + sec_row_r

    @staticmethod
    def gen_mem_usage_row(*args):
        return list(
            TfaPrettyPrinter.to_hex(arg) if type(arg) is int else arg
            for arg in args
        )

    def print_tab_mem_usage(self, modules: dict, sort_key: str = "Component"):
        click.echo("Memory Usage:")
        table = PrettyTable(
            sortby=sort_key,
            field_names=["Component", "Start", "End", "Allocated", "Free"],
        )

        for k, v in modules.items():
            start, end = v.get_mem_range()
            table.add_row(
                self.gen_mem_usage_row(
                    k.upper(), start, end, v.get_mem_size(), v.get_mem_free
                )
            )
        print(table)

    def print_mem_footprint(
        self, mem_alloc: dict, mem_free: dict, pad: int = 9
    ):
        click.echo("\nMemory Footprint:")
        for name, alloc in mem_alloc.items():
            print(
                self.get_mem_info_str(
                    name.upper(),
                    self.to_hex(alloc, padding=pad),
                    self.to_hex(mem_free[name], padding=pad),
                )
            )

    def print_virt_mem_map(
        self,
        sections: list,
        module_names: list,
        map_start: int = 12,
    ):
        click.echo("\nVirtual Address Map:")
        # Max width of the entire map always excludes the virtual address
        # column.
        col_width = int((self.term_size - map_start) / len(module_names))

        table_foot_str = re.sub(
            r"\s+$",
            "",
            "".join("{:^{}}".format(mod, col_width) for mod in module_names),
        )

        virt_addr_map = [f"{'':>{map_start}}{table_foot_str}"]

        for i, (mod, sec) in enumerate(sections):
            if sec.start != sections[i - 1][1].start or i == 1:
                pad_str = f"{self.to_hex(sec.start, padding=10):<{map_start}}"
            else:
                pad_str = f"{'':<{map_start}}"

            virt_addr_map.append(
                self.get_virt_map_row_str(
                    sec.name,
                    module_names.index(mod),
                    len(module_names),
                    width=col_width,
                    padding=pad_str,
                    border=(i == 0 or i == len(sections) - 1),
                )
            )

        virt_addr_map.reverse()
        print("\n".join(virt_addr_map))
