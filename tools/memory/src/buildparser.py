#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
from functools import cmp_to_key
import re
from elfparser import TfaElfParser


class TfaBuildParser:
    def __init__(self, path=None):
        self._modules = dict()
        self._path = path
        self.parse_modules()

    def __getitem__(self, module):
        return self._modules[module]

    def set_build_path(self, path):
        """Set the build path where the ELF binaries are found."""
        self._path = path

    def parse_modules(self):
        """Parse ELF files in the build path."""
        for _m in self._path.glob("**/*.elf"):
            module_name = _m.name.split("/")[-1].split(".")[0]
            with open(_m, "rb") as file:
                self._modules[module_name] = TfaElfParser(file)

        if not len(self._modules):
            raise FileNotFoundError(
                f"failed to find ELF files in path {self._path}!"
            )

    def get_sorted_symbols(self, expr=None) -> list:
        """Returns a map of symbols to modules."""
        expr = r".*" if not expr else expr

        symbols = [
            (*sym, k)
            for k, v in self._modules.items()
            for sym in v.get_filtered_symbols(expr)
        ]
        return sorted(symbols, key=lambda s: (-s[1], s[0]), reverse=True)

    def get_mem_usage_dict(self) -> dict:
        """Returns map of memory usage per memory type for each module."""
        mem_map = {}
        for k, v in self._modules.items():
            mod_mem_map = v.get_elf_memory_layout()
            if len(mod_mem_map):
                mem_map[k] = mod_mem_map
        return mem_map

    def get_mem_tree_as_dict(self) -> dict:
        """Returns _tree of modules, segments and segments and their total
        memory usage."""
        return {
            k: {
                "name": k,
                **v.get_mod_mem_usage_dict(),
                **{"children": v.get_seg_map_as_dict()},
            }
            for k, v in self._modules.items()
        }

    def get_sorted_mod_names(self):
        """Returns sorted list of module names."""
        return sorted(self._modules.keys())
