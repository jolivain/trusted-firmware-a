#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import os
import pytest
import yaml
from click.testing import CliRunner

from tlc.cli import cli


@pytest.fixture
def tmptfa(tmpdir):
    """Fake trusted-firmware-a directory, containing all of the blob files"""
    temp_tfa_root = tmpdir.join("trusted-firmware-a")
    os.mkdir(temp_tfa_root.strpath)
    return temp_tfa_root


@pytest.fixture
def tmptlstr(tmptfa):
    return tmptfa.join("tl.bin").strpath


@pytest.fixture
def tmpfdt(tmptfa):
    fdt = tmptfa.join("fdt.bin")
    fdt.write_binary(b"\x00" * 100)
    return fdt


@pytest.fixture
def tmpyamlconfig_fdt_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 1,
                "fdt": tmpfdt.basename,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_empty_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config_empty_entry.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 0,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_memory_layout_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config_empty_entry.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 0x104,
                "addr": 0x0400100000000010,
                "size": 0x0003300000000000,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_ep_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config_ep_entry.yaml")

    config = {
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
                    "spsr": 965,
                    "args": [67112976, 67112960, 0, 0, 0, 0, 0, 0],
                },
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_ep_entry_aarch32(tmpdir, tmpfdt):
    config_path = tmpdir.join("config_ep_entry.yaml")

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

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_hob_block_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 2,
                "hob_block": tmpfdt.basename,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_hob_list_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 3,
                "hob_list": tmpfdt.basename,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_acpi_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 4,
                "acpi_tables": tmpfdt.basename,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_spmc_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 0x101,
                "spmc_man": tmpfdt.basename,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tmpyamlconfig_optee_entry(tmpdir, tmpfdt):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": 0x100,
                "pp_addr": 100,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tlcrunner(tmptlstr):
    runner = CliRunner()
    with runner.isolated_filesystem():
        runner.invoke(cli, ["create", tmptlstr])
    return runner


@pytest.fixture
def tlc_entries(tmpfdt):
    return [(0, "/dev/null"), (1, tmpfdt.strpath), (0x102, tmpfdt.strpath)]
