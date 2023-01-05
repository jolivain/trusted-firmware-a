#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from dataclasses import dataclass


class ElfPerms:
    READ = 4
    WRITE = 2
    EXECUTE = 1


class TfaModule:
    def __init__(self, start, segments, sections, tf_map):
        self.segments = segments
        self.sections = sections
        self.start = start
        self.end = self.get_end_addr(self.segments)
        self.alloc_mem = tf_map

    @staticmethod
    def get_end_addr(segments):
        return max(map(lambda s: s.start + s.size, segments))

    @staticmethod
    def get_start_addr(segments):
        return min(map(lambda s: s.start, segments))

    def get_mem_size(self):
        return sum(map(lambda s: s.size, self.segments))

    @property
    def get_mem_free(self):
        return sum(self.alloc_mem.values()) - self.get_mem_size()

    @property
    def get_sections(self):
        return self.sections

    def get_mem_range(self):
        return self.start, self.end

    def get_mem_alloc(self):
        return self.alloc_mem

    def get_mem_type_size(self, mem_type):
        filtered_segments = filter(lambda s: s.perms == mem_type, self.segments)

        try:
            return sum(map(lambda s: s.size, filtered_segments))
        except ValueError:
            return 0


@dataclass(frozen=True)
class TfaMemObject:
    start: int
    end: int
    size: int

    def __gt__(self, other):
        return self.start > other.start


@dataclass(frozen=True)
class TfaSegment(TfaMemObject):
    perms: int = 0


@dataclass(frozen=True)
class TfaSection(TfaMemObject):
    name: str
