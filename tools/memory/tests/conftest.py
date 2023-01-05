import pytest
from src.elf import ElfPerms, TfaModule, TfaSegment, TfaSection


@pytest.fixture
def test_tfa_map():
    return {"ram": 0x12000, "rom": 0x20000}


@pytest.fixture
def test_sections():
    return [
        TfaSection(0xE0EE000, 0xE0EE000 + 0x001ADF, 0x001ADF, ".text"),
        TfaSection(0xE0EE000, 0xE0EE000 + 0x001ADF, 0x001ADF, ".rodata"),
        TfaSection(0xE0EE000, 0xE0EE000 + 0x001ADF, 0x001ADF, ".data"),
        TfaSection(0xE0EE000, 0xE0EE000 + 0x001ADF, 0x001ADF, "stacks"),
        TfaSection(0xE0EE000, 0xE0EE000 + 0x001ADF, 0x001ADF, ".bss"),
        TfaSection(0xE0B5000, 0xE0B5000 + 0x006000, 0x006000, "xlat_table"),
        TfaSection(0xE0BB000, 0xE0BB000 + 0x001000, 0x001000, "coherent_ram"),
        TfaSection(0x0000000, 0x0000000 + 0x000033, 0x000033, ".comment"),
    ]


@pytest.fixture
def test_segments():
    return [
        TfaSegment(0x0, 0x006490, 0x006490, ElfPerms.READ | ElfPerms.EXECUTE),
        TfaSegment(
            0xE0EE000,
            0xE0EE140 + 0x000105,
            0x000105,
            ElfPerms.READ | ElfPerms.WRITE,
        ),
        TfaSegment(
            0xE0EE140,
            0xE0EE140 + 0x001000,
            0x001000,
            ElfPerms.READ | ElfPerms.WRITE,
        ),
        TfaSegment(
            0xE0F0000,
            0xE0F0000 + 0x006000,
            0x006000,
            ElfPerms.READ | ElfPerms.WRITE,
        ),
        TfaSegment(
            0xE0EF140,
            0xE0F0000 + 0x000820,
            0x000820,
            ElfPerms.READ | ElfPerms.WRITE,
        ),
    ]


@pytest.fixture
def test_tfa_module(test_segments, test_sections, test_tfa_map) -> TfaModule:
    return TfaModule(0x0, test_segments, test_sections, test_tfa_map)
