#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
import re
from dataclasses import dataclass, asdict

from elftools.elf.elffile import ELFFile


@dataclass(frozen=True)
class TfaMemObject:
    name: str
    start: int
    end: int
    size: int
    children: list


class TfaElfParser:
    def __init__(self, file):
        self._segments = dict()
        self._mem_cfg = dict()

        elf = ELFFile(file)

        self._symbols = {
            sym.name: sym.entry["st_value"]
            for sym in elf.get_section_by_name(".symtab").iter_symbols()
        }

        self.set_mem_cfg()

    def set_mem_cfg(self, expr=None):
        """Retrieve information about the memory configuration from the symbol
        table.
        """
        assert len(self._symbols), "Symbol table is empty!"

        if not expr:
            expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)"
        region_symbols = dict(self.get_filtered_symbols(expr))

        for k, v in region_symbols.items():
            mem_type, _, param = tuple(k.lower().strip("__").split("_"))
            if mem_type not in self._mem_cfg.keys():
                self._mem_cfg[mem_type] = {}

            self._mem_cfg[mem_type][param] = v

    def get_mem_cfg(self):
        """Get the total memory consumed by this module from the memory
        configuration.
            {"rom": {"start": 0x0, "end": 0xFF, "length": ... }
        """
        return self._mem_cfg

    def get_filtered_symbols(self, expr):
        """Filter the symbol table by name."""
        return filter(lambda sym: re.match(expr, sym[0]), self._symbols.items())
