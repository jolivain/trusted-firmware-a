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

    def get_filtered_symbols(self, expr):
        """Filter the symbol table by name."""
        return filter(lambda sym: re.match(expr, sym[0]), self._symbols.items())
