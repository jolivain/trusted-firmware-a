#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
import collections
import functools
import operator
import re
from elf import TfaModule, TfaSegment, TfaSection, ElfPerms


# ELF encodes permissions in 3 bits [rwx], use a simple heuristic to
# figure out footprint ROM = rx, RAM = rw.

elf_mem_type = {
    "ram": ElfPerms.READ | ElfPerms.WRITE,
    "rom": ElfPerms.READ | ElfPerms.EXECUTE,
}


class MemParser:
    def __init__(self):
        self.modules = dict()

    def __getitem__(self, module):
        return self.modules[module]

    def add_module(self, elf, map_file, module_name=None):
        if not module_name:
            module_name = elf.stream.name.split("/")[-1].split(".")[0]

        elf_segments = list(elf.iter_segments())

        segments = list(
            map(
                lambda s: TfaSegment(
                    s["p_vaddr"],
                    s["p_vaddr"] + s["p_memsz"],
                    s["p_memsz"],
                    s["p_flags"],
                ),
                elf_segments,
            )
        )

        filtered_sections = filter(
            lambda s: any(seg.section_in_segment(s) for seg in elf_segments),
            elf.iter_sections(),
        )
        sections = list(
            map(
                lambda s: TfaSection(
                    s["sh_addr"],
                    s["sh_addr"] + s.data_size,
                    s.data_size,
                    s.name,
                ),
                filtered_sections,
            )
        )

        mem_config = get_mem_config_from_map(map_file)
        self.modules[module_name] = TfaModule(
            elf["e_entry"], segments, sections, mem_config
        )

    def get_mem_alloc(self) -> dict:
        mem_alloc_map = map(
            lambda mod: mod.get_mem_alloc(), self.modules.values()
        )
        mem_alloc_map = map(collections.Counter, mem_alloc_map)
        return dict(functools.reduce(operator.add, mem_alloc_map))

    def get_mem_total_free(self) -> dict:
        mem_alloc = self.get_mem_alloc()
        mem_size = self.get_mem_total_size()

        return {k: mem_alloc[k] - mem_size[k] for k in mem_alloc}

    def get_mem_total_size(self):
        mem_size = dict().fromkeys(elf_mem_type.keys())
        for mod in self.modules.values():
            for k in elf_mem_type:
                size = mod.get_mem_type_size(elf_mem_type[k])
                mem_size[k] = (
                    size if mem_size[k] is None else mem_size[k] + size
                )
        return mem_size

    def get_sorted_sections(self) -> list:
        """Returns a list of tuples composed of sections and the name of their
        module
        """
        return sorted(
            [
                (k, sec)
                for k, v in self.modules.items()
                for sec in v.get_sections
            ],
            key=lambda pair: pair[1],
        )

    def get_sorted_mod_names(self):
        return sorted(self.modules.keys())


def get_mem_config_from_map(map_file):
    re_pattern = r"(r[ao]m)(?:\s+)(0x\w+) (0x\w+)"
    mem_map = {}
    # Extract memory map from map file
    # ROM              0x0000000000000000 0x0000000004000000 xr
    # RAM              0x0000000004035000 0x000000000000b000 xrw
    for line in map_file.readlines():
        match = re.match(re_pattern, line, re.IGNORECASE)
        if match:
            mem_map[match[1].lower()] = int(match[3], 16)
        elif re.match(r"Linker script", line):
            break
    return mem_map
