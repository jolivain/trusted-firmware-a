#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from pathlib import Path

from click.testing import CliRunner

from tlc.cli import cli
from tlc.tl import TransferList


def test_create_empty_tl(tmpdir):
    runner = CliRunner()
    test_file = tmpdir.join("tl.bin")

    result = runner.invoke(cli, ["create", test_file.strpath])
    assert result.exit_code == 0
    assert TransferList.fromfile(test_file) is not None


def test_create_with_fdt(tmpdir):
    runner = CliRunner()
    fdt = tmpdir.join("fdt.bin")
    fdt.write_binary(b"\x00" * 100)

    result = runner.invoke(
        cli,
        [
            "create",
            "--fdt",
            fdt.strpath,
            "--size",
            "1000",
            tmpdir.join("tl.bin").strpath,
        ],
    )
    assert result.exit_code == 0


def test_add_single_entry(tlcrunner, tmptlstr):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 0


def test_add_multiple_entries(tlcrunner, tlc_entries, tmptlstr):
    for id, path in tlc_entries:
        tlcrunner.invoke(cli, ["add", "--entry", id, path, tmptlstr])

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == len(tlc_entries)


def test_info(tlcrunner, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])
    tlcrunner.invoke(cli, ["add", "--fdt", tmpfdt.strpath, tmptlstr])

    result = tlcrunner.invoke(cli, ["info", tmptlstr])
    assert result.exit_code == 0
    assert "signature" in result.stdout
    assert "id" in result.stdout

    result = tlcrunner.invoke(cli, ["info", "--header", tmptlstr])
    assert result.exit_code == 0
    assert "signature" in result.stdout
    assert "id" not in result.stdout

    result = tlcrunner.invoke(cli, ["info", "--entries", tmptlstr])
    assert result.exit_code == 0
    assert "signature" not in result.stdout
    assert "id" in result.stdout


def test_raises_max_size_error(tmptlstr, tmpfdt):
    tmpfdt.write_binary(bytes(6000))

    runner = CliRunner()
    result = runner.invoke(cli, ["create", "--fdt", tmpfdt, tmptlstr])

    assert result.exception
    assert isinstance(result.exception, MemoryError)
    assert "TL max size exceeded, consider increasing with the option -s" in str(
        result.exception
    )
    assert "TL size has exceeded the maximum allocation" in str(
        result.exception.__cause__
    )


def test_info_get_fdt_offset(tmptlstr, tmpfdt):
    runner = CliRunner()
    with runner.isolated_filesystem():
        runner.invoke(cli, ["create", "--size", "1000", tmptlstr])
        runner.invoke(cli, ["add", "--entry", "1", tmpfdt.strpath, tmptlstr])
        result = runner.invoke(cli, ["info", "--fdt-offset", tmptlstr])

    assert result.exit_code == 0
    assert result.output.strip("\n").isdigit()


def test_remove_tag(tlcrunner, tmptlstr):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])
    result = tlcrunner.invoke(cli, ["info", tmptlstr])

    assert result.exit_code == 0
    assert "signature" in result.stdout

    tlcrunner.invoke(cli, ["remove", "--tags", "0", tmptlstr])
    tl = TransferList.fromfile(tmptlstr)

    assert result.exit_code == 0
    assert len(tl.entries) == 0


def test_unpack_tl(tlcrunner, tmptlstr, tmpfdt, tmpdir):
    with tlcrunner.isolated_filesystem(temp_dir=tmpdir):
        tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
        tlcrunner.invoke(cli, ["unpack", tmptlstr])
        assert Path("te_1.bin").exists()


def test_unpack_multiple_tes(tlcrunner, tlc_entries, tmptlstr, tmpdir):
    with tlcrunner.isolated_filesystem(temp_dir=tmpdir):
        for id, path in tlc_entries:
            tlcrunner.invoke(cli, ["add", "--entry", id, path, tmptlstr])

    assert all(
        filter(
            lambda te: (Path(tmpdir.strpath) / f"te_{te[0]}.bin").exists(), tlc_entries
        )
    )


def test_unpack_into_dir(tlcrunner, tmpdir, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
    tlcrunner.invoke(cli, ["unpack", "-C", tmpdir.strpath, tmptlstr])

    assert (Path(tmpdir.strpath) / "te_1.bin").exists()


def test_create_fdt_entry_from_yaml(
    tlcrunner, tmpyamlconfig_fdt_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_fdt_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 1


def test_create_empty_entry_from_yaml(
    tlcrunner, tmpyamlconfig_empty_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_empty_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 0
    assert tl.entries[0].data_size == 4


def test_create_memory_layout_entry_from_yaml(
    tlcrunner, tmpyamlconfig_memory_layout_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_memory_layout_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1

    # transfer entry copied from FVP
    expected = "0x00000010 0x04001000 0x00000000 0x00033000"

    actual = tl.entries[0].data
    actual = bytes_to_hex(actual)

    assert actual == expected


def test_create_ep_entry_from_yaml(tlcrunner, tmpyamlconfig_ep_entry, tmptlstr, tmptfa):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_ep_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1

    # entry_point_info struct copied from FVP
    expected = (
        "0x00580201 0x00000008 0x04020000 0x00000000 "
        "0x000003C5 0x00000000 0x04001010 0x00000000 "
        "0x04001000 0x00000000 0x00000000 0x00000000 "
        "0x00000000 0x00000000 0x00000000 0x00000000 "
        "0x00000000 0x00000000 0x00000000 0x00000000 "
        "0x00000000 0x00000000"
    )

    actual = tl.entries[0].data
    actual = bytes_to_hex(actual)

    assert actual == expected


def test_create_ep_entry_from_yaml_aarch32(
    tlcrunner, tmpyamlconfig_ep_entry_aarch32, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_ep_entry_aarch32.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1

    # entry_point_info struct copied from FVP
    expected = (
        "0x00240201 0x00000008 0x04020000 0x000001D3 0x00000000 "
        "0x04001008 0x04001000 0x00000000 0x00000000"
    )

    actual = tl.entries[0].data
    actual = bytes_to_hex(actual)

    assert actual == expected


def test_create_hob_block_entry_from_yaml(
    tlcrunner, tmpyamlconfig_hob_block_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_hob_block_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 2


def test_create_hob_list_entry_from_yaml(
    tlcrunner, tmpyamlconfig_hob_list_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_hob_list_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 3


def test_create_acpi_entry_from_yaml(
    tlcrunner, tmpyamlconfig_acpi_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_acpi_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 4


def test_create_spmc_entry_from_yaml(
    tlcrunner, tmpyamlconfig_spmc_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_spmc_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 0x101


def test_create_optee_entry_from_yaml(
    tlcrunner, tmpyamlconfig_optee_entry, tmptlstr, tmptfa
):
    tlcrunner.invoke(
        cli,
        [
            "create-from-yaml",
            tmpyamlconfig_optee_entry.strpath,
            tmptlstr,
            "--tf-a-root",
            tmptfa.strpath,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 0x100
    assert tl.entries[0].data_size == 8

    expected_pp_addr = 100

    content = tl.entries[0].data
    actual_pp_addr = int.from_bytes(content, tl.endianness)

    assert actual_pp_addr == expected_pp_addr


def bytes_to_hex(data: bytes) -> str:
    """Convert bytes to a hex string in the same format as the debugger in
    ArmDS

    You can copy data from the debugger in Arm Development Studio and put it
    into a unit test. You can then run this function on the output from tlc,
    and compare it to the data you copied.
    """
    words_hex = []
    for i in range(0, len(data), 4):
        word = data[i : i + 4]
        word_int = int.from_bytes(word, "little")
        word_hex = "0x" + f"{word_int:0>8x}".upper()
        words_hex.append(word_hex)

    return " ".join(words_hex)
