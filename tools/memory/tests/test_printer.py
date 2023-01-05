import pytest
from src.printer import TfaPrettyPrinter


@pytest.mark.parametrize(
    ("val", "pad", "exp"),
    [
        (0, 10, "0x00000000"),
        (255, 0, "0xff"),
        (255, 10, "0x000000ff"),
        (64206, 0, "0xface"),
    ],
)
def test_to_hex(val, pad, exp):
    assert TfaPrettyPrinter.to_hex(val, pad) == exp


@pytest.mark.parametrize(
    "mem_free",
    [
        {"ram": 0, "rom": 0},
        {"ram": 255, "rom": 255},
        {"ram": 653019, "rom": 105328},
    ],
)
def test_print_mem_footprint(test_tfa_map, mem_free, capsys):
    printer = TfaPrettyPrinter()
    hex_format = printer.to_hex
    pad = 9

    exp = "\nMemory Footprint:\n"
    for k, v in mem_free.items():
        exp += f"Total {k.upper()} memory: {hex_format(test_tfa_map[k], padding=pad)} bytes [free = {hex_format(v, padding=pad)}]\n"

    printer.print_mem_footprint(test_tfa_map, mem_free, pad=pad)
    assert capsys.readouterr().out == exp


def test_print_tab_mem_usage(test_tfa_module, capsys):
    exp = """
    +-----------+------------+------------+------------+------------+
    | Component |   Start    |    End     | Allocated  |    Free    |
    +-----------+------------+------------+------------+------------+
    |    BL1    | 0x00000000 | 0x0e0f6000 | 0x0000ddb5 | 0x0002424b |
    |    BL2    | 0x00000000 | 0x0e0f6000 | 0x0000ddb5 | 0x0002424b |
    +-----------+------------+------------+------------+------------+
    """
    printer = TfaPrettyPrinter()
    printer.print_tab_mem_usage(
        {"bl1": test_tfa_module, "bl2": test_tfa_module}
    )
    assert capsys.readouterr().out
