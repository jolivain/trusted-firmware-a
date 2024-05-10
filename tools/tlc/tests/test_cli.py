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


def test_info_get_fdt_offset(tmptlstr, tmpfdt):
    runner = CliRunner()
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


def test_unpack_tl(tlcrunner, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
    tlcrunner.invoke(cli, ["unpack", tmptlstr])
    assert Path("te_1.bin").exists()


def test_unpack_multiple_tes(tlcrunner, tlc_entries, tmptlstr, tmpdir):
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
