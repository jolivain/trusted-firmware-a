#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
import re
from elftools.elf.elffile import ELFFile


class TfaElfParser:
    def __init__(self, file):
        self._segments = dict()
        self._memory_layout = dict()

        elf = ELFFile(file)

        self._symbols = {
            sym.name: sym.entry["st_value"]
            for sym in elf.get_section_by_name(".symtab").iter_symbols()
        }

        self._memory_layout = self.get_memory_layout_from_symbols()

    def get_memory_layout_from_symbols(self, expr=None) -> dict:
        """Retrieve information about the memory configuration from the symbol
        table.
        """
        assert len(self._symbols), "Symbol table is empty!"

        if not expr:
            expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)"

        region_symbols = dict(self.get_filtered_symbols(expr))
        memory_layout = {}

        for k, v in region_symbols.items():
            region, _, attr = tuple(k.lower().strip("__").split("_"))
            if region not in memory_layout:
                memory_layout[region] = {}

            memory_layout[region][attr] = v

        return memory_layout

    def get_elf_memory_layout(self):
        """Get the total memory consumed by this module from the memory
        configuration.
            {"rom": {"start": 0x0, "end": 0xFF, "length": ... }
        """
        mem_dict = {}

        for mem, attrs in self._memory_layout.items():
            limit = attrs["start"] + attrs["length"]
            mem_dict[mem] = {
                "start": attrs["start"],
                "limit": limit,
                "size": attrs["end"] - attrs["start"],
                "free": limit - attrs["end"],
                "total":  attrs["length"]
            }
        return mem_dict

    def get_filtered_symbols(self, expr):
        """Filter the symbol table by name."""
        return filter(lambda sym: re.match(expr, sym[0]), self._symbols.items())
