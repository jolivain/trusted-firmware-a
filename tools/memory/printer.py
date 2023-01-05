#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
import re

import click
from prettytable import PrettyTable


class TfaPrettyPrinter:
    def __init__(self, parser):
        self.parser = parser

    @staticmethod
    def to_hex(val, padding=10):
        return f"{val:#0{padding}x}"

    @staticmethod
    def get_mem_info_str(name, size, free):
        return f"Total {name} memory: {size} bytes [free = {free}] "

    @staticmethod
    def get_map_row_str(
        section_name, rel_pos, columns, width=None, padding=None, border=False
    ):
        empty_col = "{:{}{}}"
        sec_row = f"+{section_name:-^{width-1}}+"
        if border:
            sec_row_l = empty_col.format("+", "-<", width) * rel_pos
            sec_row_r = empty_col.format("+", "->", width) * (
                columns - rel_pos - 1
            )
        else:
            sec_row_l = empty_col.format("|", "<", width) * rel_pos
            sec_row_r = empty_col.format("|", ">", width) * (
                columns - rel_pos - 1
            )

        return padding + sec_row_l + sec_row + sec_row_r

    def gen_table_row(self, *args):
        return list(
            self.to_hex(arg) if type(arg) is int else arg for arg in args
        )

    def print_table(self, sort_key="Component"):
        click.echo("Memory Usage:")
        table = PrettyTable()
        table.field_names = ["Component", "Start", "End", "Size", "Free"]
        table.sortby = sort_key

        for k, v in self.parser.modules.items():
            start, end = v.get_mem_range()
            table.add_row(
                self.gen_table_row(
                    k.upper(),
                    start,
                    end,
                    v.get_mem_size(),
                    v.get_mem_free,
                )
            )
        print(table)

    def print_mem_footprint(self):
        click.echo("\nMemory Footprint:")
        mem_alloc = self.parser.get_mem_alloc()

        for mem, size in self.parser.get_mem_total_size().items():
            print(
                self.get_mem_info_str(
                    mem.upper(),
                    self.to_hex(size, padding=8),
                    self.to_hex(mem_alloc[mem] - size, padding=9),
                )
            )

    def print_mem_map(self, max_term_len=120, map_start=12):
        click.echo("\nVirtual Address Map:")
        mod_sec_dict = self.parser.get_all_sections()
        mod_names = list(mod_sec_dict.keys())
        mod_names.sort()

        # Transform dictionary of module to section mappings into a tuple that
        # we can sort by start address, while retaining module info.
        sorted_sections = [
            (mod, s) for mod, sections in mod_sec_dict.items() for s in sections
        ]
        sorted_sections.sort(key=lambda s: s[1].start)

        # Max width of the entire map always excludes the virtual address
        # column.
        col_width = int((max_term_len - map_start) / len(mod_names))

        table_foot_str = re.sub(
            r"\s+$",
            "",
            "".join("{:^{}}".format(mod, col_width) for mod in mod_names),
        )

        map_str = [f"{'':>{map_start}}{table_foot_str}"]

        for i, (mod, sec) in enumerate(sorted_sections):
            if sec.start != sorted_sections[i - 1][1].start or i == 1:
                pad_str = f"{self.to_hex(sec.start, padding=10):<{map_start}}"
            else:
                pad_str = f"{'':<{map_start}}"

            map_str.append(
                self.get_map_row_str(
                    sec.name,
                    mod_names.index(mod),
                    len(mod_names),
                    width=col_width,
                    padding=pad_str,
                    border=(i == 0 or i == len(sorted_sections) - 1),
                )
            )

        map_str.reverse()
        print("\n".join(map_str))
