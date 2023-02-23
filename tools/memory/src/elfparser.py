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

    def get_filtered_symbols(self, expr):
        """Filter the symbol table by name."""
        return filter(lambda sym: re.match(expr, sym[0]), self._symbols.items())
