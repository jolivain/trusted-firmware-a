#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import typing

import math
import struct
from dataclasses import dataclass
from pathlib import Path

from tlc.te import TransferEntry

TRANSFER_LIST_ENABLE_CHECKSUM = 0b1


class TransferList:
    """Class representing a Transfer List based on version 1.0 of the Firmware Handoff specification."""

    tl_hdr_size = 0x18
    endianness = "little"

    def __init__(
        self, max_size: int = tl_hdr_size, flags: int = TRANSFER_LIST_ENABLE_CHECKSUM
    ) -> None:
        assert max_size >= self.tl_hdr_size
        self.signature: int = 0x4A0FB10B
        self.checksum: int = 0
        self.version: int = 1
        self.hdr_size = self.tl_hdr_size
        self.alignment: int = 3
        self.size = self.tl_hdr_size
        self.total_size = max_size
        self.flags = flags
        self.entries: typing.List["TransferEntry"] = []
        self.update_checksum()

    def __str__(self) -> str:
        return "\n".join(
            [
                f"{k:<10} {hex(v)}"
                for k, v in vars(self).items()
                if not isinstance(v, list)
            ]
        )

    def get_transfer_entries_str(self):
        return "\n----\n".join([str(te) for _, te in enumerate(self.entries)])

    @classmethod
    def fromfile(cls, filepath: Path) -> "TransferList":
        tl = cls()

        with open(filepath, "rb") as f:
            (
                tl.signature,
                tl.checksum,
                tl.version,
                tl.hdr_size,
                tl.alignment,
                used_size,
                tl.total_size,
                tl.flags,
                _,
            ) = struct.unpack(
                ("<" if cls.endianness == "little" else ">") + "I4B4I",
                f.read(tl.tl_hdr_size),
            )

            while tl.size < used_size:
                # We add an extra padding byte into the header so we can extract
                # the 3-byte wide ID as a 4-byte uint, shift out this padding
                # once we have the id.
                te_base = f.tell()
                (id, hdr_size, data_size) = struct.unpack(
                    ("<" if cls.endianness == "little" else ">") + "IBI",
                    b"\x00" + f.read(TransferEntry.hdr_size),
                )

                id >>= 8

                te = tl.add_transfer_entry(id, f.read(data_size))
                te.offset = te_base
                f.seek(align(te_base + hdr_size + data_size, 2**tl.alignment))

        return tl

    @classmethod
    def from_dict(cls, config: dict, tf_a_root: Path):
        """Create a TL from data in a dictionary

        For example:
        config = {
            "execution_state": "aarch32",
            "has_checksum": True,
            "max_size": 0x1000,
            "entries": [
                {
                    "tag_id": 0x102,
                    "ep_info": {
                        "h": {
                            "type": 0x01,
                            "version": 0x02,
                            "attr": 8,
                        },
                        "pc": 67239936,
                        "spsr": 467,
                        "lr_svc": 0,
                        "args": [67112968, 67112960, 0, 0],
                    },
                },
            ],
        }

        max_size defaults to 0x1000, execution_state defaults to aarch64, and
        has_checksum defaults to True.

        See TransferList.add_transfer_entry_from_dict for more detail.

        :param config: Dictionary containing the data described above.
        :param tf_a_root: Path to the root of the TF-A repository. Needed
        because blob file paths are relative to the root of TF-A.
        """
        # get settings from config and set defaults
        max_size = config.get("max_size", 0x1000)
        execution_state = config.get("execution_state", "aarch64")
        has_checksum = config.get("has_checksum", True)

        print(f"WARNING: Using {execution_state}.")

        # set flags
        flags = TRANSFER_LIST_ENABLE_CHECKSUM if has_checksum else 0

        # create TL
        tl = cls(max_size, flags)

        # add entries
        try:
            for entry in config["entries"]:
                tl.add_transfer_entry_from_dict(entry, tf_a_root, execution_state)
        except MemoryError as mem_excp:
            raise MemoryError(
                "TL max size exceeded, consider increasing with the option -s"
            ) from mem_excp

        return tl

    def header_to_bytes(self) -> bytes:
        return struct.pack(
            ("<" if self.endianness == "little" else ">") + "I4B4I",
            self.signature,
            self.checksum,
            self.version,
            self.hdr_size,
            self.alignment,
            self.size,
            self.total_size,
            self.flags,
            0,
        )

    def update_checksum(self) -> None:
        """Calculates the checksum based on the sum of bytes."""
        self.checksum = 256 - ((self.sum_of_bytes() - self.checksum) % 256)

    def sum_of_bytes(self) -> int:
        """Sum of all bytes between the base address and the end of that last TE."""
        _csum = 0
        for _, v in self.__dict__.items():
            if isinstance(v, list):
                _csum += sum(te.sum_of_bytes for te in v)
            else:
                _csum += sum(v.to_bytes(4, self.endianness))
        return _csum % 256

    def get_entry_data_offset(self, tag_id: int) -> int:
        for te in self.entries:
            if te.id == tag_id:
                return te.offset + te.hdr_size

        raise ValueError(f"Tag {tag_id} not found in TL!")

    def add_transfer_entry(self, tag_id: int, data: bytes) -> "TransferEntry":
        """Appends a TransferEntry into the internal list of TE's."""
        if not (self.total_size >= self.size + TransferEntry.hdr_size + len(data)):
            raise MemoryError(
                f"TL size has exceeded the maximum allocation {self.total_size}."
            )
        else:
            te = TransferEntry(tag_id, len(data), data)
            self.entries.append(te)
            self.size += te.size
            self.update_checksum()
            return te

    def add_empty_transfer_entry(self) -> "TransferEntry":
        return self.add_transfer_entry(0, bytes(4))

    def add_fdt_transfer_entry(self, fdt_file: Path) -> "TransferEntry":
        return self.add_transfer_entry_from_file(1, fdt_file)

    def add_hob_block_transfer_entry(self, hob_block_file: Path) -> "TransferEntry":
        return self.add_transfer_entry_from_file(2, hob_block_file)

    def add_hob_list_transfer_entry(self, hob_list_file: Path) -> "TransferEntry":
        return self.add_transfer_entry_from_file(3, hob_list_file)

    def add_acpi_table_aggregate_transfer_entry(
        self, acpi_tables_file: Path
    ) -> "TransferEntry":
        return self.add_transfer_entry_from_file(4, acpi_tables_file)

    def add_event_log_table_transfer_entry(self, flags: int, event_log: Path):
        with open(event_log, "rb") as f:
            event_log_data = f.read()

        flags_bytes = flags.to_bytes(4, self.endianness)
        data = flags_bytes + event_log_data

        return self.add_transfer_entry(5, data)

    def add_tpm_crb_base_address_transfer_entry(
        self, crb_base_address: int, crb_size: int
    ) -> "TransferEntry":
        struct_format = ("<" if self.endianness == "little" else ">") + "QI"
        data = struct.pack(
            struct_format,
            crb_base_address,
            crb_size,
        )

        return self.add_transfer_entry(6, data)

    def add_op_tee_transfer_entry(self, pp_addr: int) -> "TransferEntry":
        struct_format = ("<" if self.endianness == "little" else ">") + "Q"
        data = struct.pack(
            struct_format,
            pp_addr,
        )

        return self.add_transfer_entry(0x100, data)

    def add_spmc_manifest_transfer_entry(self, smpc_man_file: Path) -> "TransferEntry":
        return self.add_transfer_entry_from_file(0x101, smpc_man_file)

    def add_memory_layout_transfer_entry(self, addr: int, size: int):
        struct_format = ("<" if self.endianness == "little" else ">") + "QQ"
        data = struct.pack(
            struct_format,
            addr,
            size,
        )

        return self.add_transfer_entry(0x104, data)

    def add_entry_point_info_transfer_entry(
        self,
        struct_type: int,
        version: int,
        attr: int,
        pc: int,
        spsr: int,
        args: typing.List[int],
        execution_state: typing.Literal["aarch64", "aarch32"],
        lr_svc: typing.Optional[int] = None,
    ) -> "TransferEntry":
        assert execution_state in ("aarch64", "aarch32")

        # size of the entry_point_info struct
        entry_point_size = 88 if execution_state == "aarch64" else 36

        # define the struct format
        struct_format = (
            "BBHIQIxxxxQQQQQQQQ" if execution_state == "aarch64" else "BBHIIIIIIII"
        )

        # specify endianness
        endian_op = "<" if self.endianness == "little" else ">"
        struct_format = endian_op + struct_format

        # define list of items in the struct
        items = [
            struct_type,
            version,
            entry_point_size,
            attr,
            pc,
            spsr,
            *args,
        ]
        if execution_state == "aarch32":
            assert lr_svc is not None
            items.insert(6, lr_svc)

        # pack struct
        data = struct.pack(
            struct_format,
            *items,
        )

        return self.add_transfer_entry(0x102, data)

    def add_transfer_entry_from_dict(
        self,
        entry: dict,
        tf_a_root: Path,
        execution_state: typing.Literal["aarch64", "aarch32"],
    ) -> "TransferEntry":
        """Add a transfer entry from data in a dictionary

        The fields of the dictionary should match the fields in the
        specification for the transfer list. You don't need to give the
        hdr_size or data_size fields. For example, a memory layout entry would
        have a dictionary like:
        {
            "tag_id": 0x3,
            "addr": 0x8,
            "size": 0x8,
        }

        You can input blob files by giving paths relative to the root of the
        TF-A repository. For example, an FDT layout would have a dictionary
        like:
        {
            tag_id: 1,
            fdt: "fdt.bin",
        }

        You can input C-types by giving its fields. For example, an entry point
        info entry would have a dictionary like:
        {
            "tag_id": 0x102,
            "ep_info": {
                "h": {
                    "type": 0x01,
                    "version": 0x02,
                    "attr": 8,
                },
                "pc": 67239936,
                "spsr": 965,
                "args": [67112976, 67112960, 0, 0, 0, 0, 0, 0]
            }
        }

        :param entry: Dictionary containing the data described above.
        :param tf_a_root: Path to the root of the TF-A repository. Needed
        because blob file paths are relative to the root of TF-A.
        :param execution_state: aarch32 or aarch64. Needed because some data
        structures have different formats depending on the execution state.
        """
        tag_id = entry["tag_id"]

        if tag_id == 0:
            return self.add_empty_transfer_entry()
        elif tag_id == 1:
            return self.add_fdt_transfer_entry(tf_a_root / entry["fdt"])
        elif tag_id == 2:
            return self.add_hob_block_transfer_entry(tf_a_root / entry["hob_block"])
        elif tag_id == 3:
            return self.add_hob_list_transfer_entry(tf_a_root / entry["hob_list"])
        elif tag_id == 4:
            return self.add_acpi_table_aggregate_transfer_entry(
                tf_a_root / entry["acpi_tables"]
            )
        elif tag_id == 5:
            return self.add_event_log_table_transfer_entry(
                entry["flags"], tf_a_root / entry["event_log"]
            )
        elif tag_id == 6:
            return self.add_tpm_crb_base_address_transfer_entry(
                entry["crb_base_address"], entry["crb_size"]
            )
        elif tag_id == 0x100:
            return self.add_op_tee_transfer_entry(entry["pp_addr"])
        elif tag_id == 0x101:
            return self.add_spmc_manifest_transfer_entry(tf_a_root / entry["spmc_man"])
        elif tag_id == 0x102:
            ep_info = entry["ep_info"]
            header = ep_info["h"]
            return self.add_entry_point_info_transfer_entry(
                header["type"],
                header["version"],
                header["attr"],
                ep_info["pc"],
                ep_info["spsr"],
                ep_info["args"],
                execution_state,
                ep_info.get("lr_svc", None),
            )
        elif tag_id == 0x104:
            return self.add_memory_layout_transfer_entry(entry["addr"], entry["size"])
        else:
            raise ValueError(f"Invalid tag id {tag_id}.")

    def add_transfer_entry_from_file(self, tag_id: int, path: Path) -> "TransferEntry":
        with open(path, "rb") as f:
            return self.add_transfer_entry(tag_id, f.read())

    def write_to_file(self, file: Path) -> None:
        """Write the contents of the TL to a file."""
        with open(file, "wb") as f:
            f.write(self.header_to_bytes())
            for te in self.entries:
                assert f.tell() + te.hdr_size + te.data_size < self.total_size
                te_base = f.tell()
                f.write(te.header_to_bytes(endianness=self.endianness))
                f.write(te.data)
                # Ensure the next TE has the correct alignment
                f.write(
                    bytes(
                        (
                            align(
                                te_base + te.hdr_size + te.data_size, 2**self.alignment
                            )
                            - f.tell()
                        )
                    )
                )

    def remove_tag(self, tag: int) -> None:
        self.entries = list(filter(lambda te: te.id != tag, self.entries))
        self.size = self.hdr_size + sum(map(lambda te: te.size, self.entries))
        self.update_checksum()


def align(n, alignment):
    return int(math.ceil(n / alignment) * alignment)
