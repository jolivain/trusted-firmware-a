#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from re import search
from typing import TextIO


class TfaMapParser:
    """A class representing a map file built for TF-A.

    Provides a basic interface for reading the symbol table. The constructor
    accepts a file-like object with the contents a Map file. Only GNU map files
    are supported at this stage.
    """

    def __init__(self, map_file: TextIO):
        self._symbols = self.read_symbols(map_file)

    @property
    def symbols(self):
        return self._symbols.items()

    @staticmethod
    def read_symbols(file: TextIO, pattern: str = None) -> dict:
        pattern = r"\b(0x\w*)\s*(\w*)\s=" if not pattern else pattern
        symbols = {}

        for line in file.readlines():
            match = search(pattern, line)

            if match is not None:
                value, name = match.groups()
                symbols[name] = int(value, 16)

        return symbols
