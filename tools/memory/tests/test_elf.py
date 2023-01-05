import pytest
from src.elf import ElfPerms


def test_get_mem_size(test_tfa_module, test_segments):
    exp = sum([s.size for s in test_segments])
    assert test_tfa_module.get_mem_size() == exp


def test_get_mem_range(test_tfa_module):
    start, end = test_tfa_module.get_mem_range()
    assert start == 0x0
    assert end == 0x0E0F6000


def test_get_mem_free(test_tfa_module, test_segments, test_tfa_map):
    mem_alloc_total = sum(test_tfa_map.values())
    mem_size = sum([s.size for s in test_segments])
    assert test_tfa_module.get_mem_free == mem_alloc_total - mem_size


@pytest.mark.parametrize(
    "perms",
    [
        ElfPerms.READ | ElfPerms.WRITE | ElfPerms.EXECUTE,
        ElfPerms.READ | ElfPerms.EXECUTE,
        ElfPerms.READ | ElfPerms.WRITE,
    ],
)
def test_get_mem_type_size(test_tfa_module, test_segments, perms):
    exp = sum(
        [
            seg.size
            for seg in filter(lambda seg: seg.perms == perms, test_segments)
        ]
    )
    assert test_tfa_module.get_mem_type_size(perms) == exp
