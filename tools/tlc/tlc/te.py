#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import struct
from dataclasses import dataclass


@dataclass
class TransferEntry:
    """Class representing a Transfer Entry."""

    id: int
    data_size: int
    data: bytes
    hdr_size: int = 8
    offset: int = 0

    def __str__(self) -> str:
        return "\n".join(
            [
                f"{k:<10} {hex(v)}"
                for k, v in vars(self).items()
                if not isinstance(v, bytes)
            ]
        )

    @property
    def size(self) -> int:
        return self.hdr_size + len(self.data)

    @property
    def sum_of_bytes(self) -> int:
        return (sum(self.header_to_bytes(endianness="little")) + sum(self.data)) % 256

    def header_to_bytes(self, endianness: str = "little") -> bytes:
        format = ("<" if endianness == "little" else ">") + "BI"
        return self.id.to_bytes(3, endianness) + struct.pack(
            format, self.hdr_size, self.data_size
        )
