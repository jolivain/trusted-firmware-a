#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
from anytree import RenderTree
from anytree.importer import DictImporter
from prettytable import PrettyTable


class TfaPrettyPrinter:
    def __init__(self, columns=None, num_as_hex=True):
        self.term_size = columns if columns and columns > 120 else 120
        self._symbol_map = None
        self._num_as_hex = num_as_hex

    def format_args(self, *args, width=10, fmt=None):
        if not fmt and type(args[0]) is int:
            fmt = f">{width}x" if self._num_as_hex else f">{width}"
        return [f"{arg:{fmt}}" if fmt else arg for arg in args]

    @staticmethod
    def map_elf_symbol(
        leading, section_name, rel_pos, columns, width=None, border=False
    ):
        empty_col = "{:{}{}}"

        # Some symbols are longer than the column width, truncate them until
        # we find a more elegant way to display them!
        len_over = len(section_name) - width
        if len_over > 0:
            section_name = section_name[len_over:-len_over]

        sec_row = f"+{section_name:-^{width-1}}+"
        sep, fill = ("+", "-") if border else ("|", "")

        sec_row_l = empty_col.format(sep, fill + "<", width) * rel_pos
        sec_row_r = empty_col.format(sep, fill + ">", width) * (
            columns - rel_pos - 1
        )

        return leading + sec_row_l + sec_row + sec_row_r

    def print_symbol_table(
        self,
        symbols: list,
        modules: list,
        start: int = 11,
    ):
        assert len(symbols), "Empty symbol list!"
        modules = sorted(modules)
        col_width = int((self.term_size - start) / len(modules))

        num_fmt = f"0=#010x" if self._num_as_hex else f">10"

        _symbol_map = [
            " " * start
            + "".join(self.format_args(*modules, fmt=f"^{col_width}"))
        ]
        last_addr = None

        for i, (name, addr, mod) in enumerate(symbols):
            # Do not print out an address twice if two symbols overlap,
            # for example, at the end of one region and start of another.
            leading = (
                f"{addr:{num_fmt}}" + " " if addr != last_addr else " " * start
            )

            _symbol_map.append(
                self.map_elf_symbol(
                    leading,
                    name,
                    modules.index(mod),
                    len(modules),
                    width=col_width,
                    border=(not i or i == len(symbols) - 1),
                )
            )

            last_addr = addr

        self._symbol_map = ["Virtual Address Map:"]
        self._symbol_map += list(reversed(_symbol_map))
        print("\n".join(self._symbol_map))
